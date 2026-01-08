#include "shader.hpp"
#include "vulkan_context.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <filesystem>

Shader::~Shader() {
    // Note: cleanup() must be called explicitly before destruction
    // This is consistent with other Vulkan classes in the project
}

Shader::Shader(Shader&& other) noexcept
    : m_shaderModule(other.m_shaderModule)
    , m_stage(other.m_stage)
    , m_path(std::move(other.m_path))
    , m_lastWriteTime(other.m_lastWriteTime)
    , m_initialized(other.m_initialized)
    , m_isGLSL(other.m_isGLSL)
{
    other.m_shaderModule = vk::ShaderModule();
    other.m_initialized = false;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        m_shaderModule = other.m_shaderModule;
        m_stage = other.m_stage;
        m_path = std::move(other.m_path);
        m_lastWriteTime = other.m_lastWriteTime;
        m_initialized = other.m_initialized;
        m_isGLSL = other.m_isGLSL;

        other.m_shaderModule = vk::ShaderModule();
        other.m_initialized = false;
    }
    return *this;
}

bool Shader::loadFromSPIRV(VulkanContext& context, const std::string& path, Stage stage) {
    if (m_initialized) {
        SPDLOG_ERROR("Shader: Already initialized");
        return false;
    }

    m_stage = stage;
    m_path = path;
    m_isGLSL = false;

    std::vector<uint32_t> spirvCode;
    if (!loadSPIRVFile(path, spirvCode)) {
        return false;
    }

    if (!createShaderModule(context, spirvCode)) {
        return false;
    }

    m_initialized = true;
    updateLastWriteTime();
    SPDLOG_INFO("Shader: Loaded SPIR-V shader: {} ({})", path, getStageName(stage));
    return true;
}

bool Shader::loadFromGLSL(VulkanContext& context, const std::string& path, Stage stage) {
    if (m_initialized) {
        SPDLOG_ERROR("Shader: Already initialized");
        return false;
    }

    m_stage = stage;
    m_path = path;
    m_isGLSL = true;

    // Try to find corresponding .spv file
    // The compiled shader file is named <basename>.<ext>.spv (e.g., basic.vert.spv)
    std::filesystem::path glslPath(path);
    std::filesystem::path spirvPath = glslPath.parent_path() / (glslPath.filename().string() + ".spv");

    // Check if SPIR-V exists
    bool usePrecompiled = std::filesystem::exists(spirvPath);

    std::vector<uint32_t> spirvCode;

    if (usePrecompiled) {
        SPDLOG_INFO("Shader: Using pre-compiled SPIR-V: {}", spirvPath.string());
        if (!loadSPIRVFile(spirvPath.string(), spirvCode)) {
            SPDLOG_ERROR("Shader: Failed to load pre-compiled SPIR-V");
            return false;
        }
    } else {
        SPDLOG_ERROR("Shader: SPIR-V file not found: {}", spirvPath.string());
        SPDLOG_ERROR("Shader: Run: glslangValidator -V {} -o {}", path, spirvPath.string());
        return false;
    }

    if (!createShaderModule(context, spirvCode)) {
        return false;
    }

    m_initialized = true;
    updateLastWriteTime();
    SPDLOG_INFO("Shader: Loaded GLSL shader: {} ({})", path, getStageName(stage));
    return true;
}

bool Shader::checkForReload(VulkanContext& context) {
    if (!m_initialized || !m_isGLSL) {
        return false;
    }

    std::error_code ec;
    auto currentTime = std::filesystem::last_write_time(m_path, ec);
    if (ec) {
        return false;
    }

    if (currentTime > m_lastWriteTime) {
        SPDLOG_INFO("Shader: Detected change in {}, reloading...", m_path);

        // Save old state
        vk::ShaderModule oldModule = m_shaderModule;
        bool oldInitialized = m_initialized;
        std::string oldPath = m_path;

        // Try to reload
        m_initialized = false;
        m_shaderModule = vk::ShaderModule();

        if (loadFromGLSL(context, oldPath, m_stage)) {
            // Success - destroy old module
            context.getDevice().destroyShaderModule(oldModule);
            return true;
        } else {
            // Failure - restore old state
            m_shaderModule = oldModule;
            m_initialized = oldInitialized;
            m_path = oldPath;
            SPDLOG_ERROR("Shader: Failed to reload shader, keeping previous version");
            return false;
        }
    }

    return false;
}

void Shader::cleanup(VulkanContext& context) {
    if (m_shaderModule) {
        context.getDevice().destroyShaderModule(m_shaderModule);
        m_shaderModule = vk::ShaderModule();
    }

    m_initialized = false;
    m_path.clear();
    m_isGLSL = false;
}

vk::ShaderStageFlagBits Shader::getVkStage() const {
    switch (m_stage) {
        case Stage::Vertex:          return vk::ShaderStageFlagBits::eVertex;
        case Stage::Fragment:        return vk::ShaderStageFlagBits::eFragment;
        case Stage::Geometry:        return vk::ShaderStageFlagBits::eGeometry;
        case Stage::TessControl:     return vk::ShaderStageFlagBits::eTessellationControl;
        case Stage::TessEvaluation:  return vk::ShaderStageFlagBits::eTessellationEvaluation;
        case Stage::Compute:         return vk::ShaderStageFlagBits::eCompute;
        default:                     return vk::ShaderStageFlagBits::eVertex;
    }
}

const char* Shader::getStageExtension(Stage stage) {
    switch (stage) {
        case Stage::Vertex:          return ".vert";
        case Stage::Fragment:        return ".frag";
        case Stage::Geometry:        return ".geom";
        case Stage::TessControl:     return ".tesc";
        case Stage::TessEvaluation:  return ".tese";
        case Stage::Compute:         return ".comp";
        default:                     return ".vert";
    }
}

const char* Shader::getStageName(Stage stage) {
    switch (stage) {
        case Stage::Vertex:          return "vertex";
        case Stage::Fragment:        return "fragment";
        case Stage::Geometry:        return "geometry";
        case Stage::TessControl:     return "tess_control";
        case Stage::TessEvaluation:  return "tess_evaluation";
        case Stage::Compute:         return "compute";
        default:                     return "unknown";
    }
}

bool Shader::createShaderModule(VulkanContext& context, const std::vector<uint32_t>& spirvCode) {
    if (spirvCode.empty()) {
        SPDLOG_ERROR("Shader: Empty SPIR-V code");
        return false;
    }

    // Validate SPIR-V magic number
    if (spirvCode[0] != 0x07230203) {
        SPDLOG_ERROR("Shader: Invalid SPIR-V magic number: {:#x}", spirvCode[0]);
        return false;
    }

    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.setCodeSize(spirvCode.size() * sizeof(uint32_t));
    createInfo.setPCode(spirvCode.data());

    vk::Result result = context.getDevice().createShaderModule(&createInfo, nullptr, &m_shaderModule);
    if (result != vk::Result::eSuccess) {
        SPDLOG_ERROR("Shader: Failed to create shader module: {}", static_cast<int>(result));
        return false;
    }

    return true;
}

bool Shader::loadSPIRVFile(const std::string& path, std::vector<uint32_t>& outCode) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        SPDLOG_ERROR("Shader: Failed to open file: {}", path);
        return false;
    }

    size_t fileSize = file.tellg();
    file.seekg(0);

    // SPIR-V is stored as 32-bit words
    if (fileSize % sizeof(uint32_t) != 0) {
        SPDLOG_ERROR("Shader: Invalid SPIR-V file size: {} (not a multiple of 4)", fileSize);
        return false;
    }

    size_t wordCount = fileSize / sizeof(uint32_t);
    outCode.resize(wordCount);
    file.read(reinterpret_cast<char*>(outCode.data()), fileSize);

    if (!file) {
        SPDLOG_ERROR("Shader: Failed to read file: {}", path);
        return false;
    }

    return true;
}

void Shader::updateLastWriteTime() {
    std::error_code ec;
    m_lastWriteTime = std::filesystem::last_write_time(m_path, ec);
    if (ec) {
        SPDLOG_WARN("Shader: Failed to get last write time for: {}", m_path);
    }
}
