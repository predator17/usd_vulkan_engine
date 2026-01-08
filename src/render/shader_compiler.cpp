#include "shader_compiler.hpp"
#include "shader.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <array>
#include <cstdio>

// Static members
std::string ShaderCompiler::s_glslangValidatorPath;
bool ShaderCompiler::s_compilerChecked = false;

bool ShaderCompiler::findGlslangValidator() {
    if (s_compilerChecked) {
        return !s_glslangValidatorPath.empty();
    }

    s_compilerChecked = true;

    // Common paths for glslangValidator
    const std::array<const char*, 6> possiblePaths = {
        "glslangValidator",
        "/usr/bin/glslangValidator",
        "/usr/local/bin/glslangValidator",
        "C:/VulkanSDK/*/Bin/glslangValidator.exe",
        "/opt/VulkanSDK/*/bin/glslangValidator",
        nullptr
    };

    for (const char* path : possiblePaths) {
        if (!path) break;

        // Check if file exists directly
        std::filesystem::path fsPath(path);
        if (std::filesystem::exists(fsPath)) {
            s_glslangValidatorPath = path;
            SPDLOG_INFO("ShaderCompiler: Found glslangValidator at: {}", path);
            return true;
        }
    }

    SPDLOG_WARN("ShaderCompiler: glslangValidator not found in system PATH");
    SPDLOG_WARN("ShaderCompiler: Shader runtime compilation will not work");
    SPDLOG_WARN("ShaderCompiler: Install Vulkan SDK to enable these features");
    return false;
}

bool ShaderCompiler::isCompilerAvailable() {
    findGlslangValidator();
    return !s_glslangValidatorPath.empty();
}

ShaderCompiler::CompileResult ShaderCompiler::compileGLSL(const std::string& glslCode, Shader::Stage stage) {
    if (!isCompilerAvailable()) {
        return CompileResult::error("glslangValidator not found");
    }

    // Create temporary file for GLSL source
    std::filesystem::path tempPath = std::filesystem::temp_directory_path() / "usd_ve_temp_XXXXXX";
    std::string glslFile = tempPath.string() + Shader::getStageExtension(stage);
    std::string spirvFile = tempPath.string() + ".spv";

    // Write GLSL to temp file
    {
        std::ofstream out(glslFile);
        out << glslCode;
        if (!out) {
            return CompileResult::error("Failed to write temporary GLSL file");
        }
    }

    // Compile
    auto result = compileGLSLFile(glslFile, stage);

    // Clean up temp files
    std::filesystem::remove(glslFile);
    if (result.success) {
        std::filesystem::remove(spirvFile);
    }

    return result;
}

ShaderCompiler::CompileResult ShaderCompiler::compileGLSLFile(const std::string& path, Shader::Stage stage) {
    if (!isCompilerAvailable()) {
        return CompileResult::error("glslangValidator not found. Please install Vulkan SDK.");
    }

    // Read GLSL file
    std::ifstream file(path);
    if (!file.is_open()) {
        return CompileResult::error("Failed to open GLSL file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string glslCode = buffer.str();

    // Create temp SPIR-V file path
    std::filesystem::path glslPath(path);
    std::string spirvPath = (glslPath.parent_path() / (glslPath.stem().string() + "_temp.spv")).string();

    // Compile and save
    if (!compileAndSave(path, spirvPath, stage)) {
        return CompileResult::error("Failed to compile GLSL file");
    }

    // Read SPIR-V
    std::ifstream spirvFile(spirvPath, std::ios::binary | std::ios::ate);
    if (!spirvFile.is_open()) {
        return CompileResult::error("Failed to open compiled SPIR-V file");
    }

    size_t fileSize = spirvFile.tellg();
    spirvFile.seekg(0);

    std::vector<uint32_t> spirvCode(fileSize / sizeof(uint32_t));
    spirvFile.read(reinterpret_cast<char*>(spirvCode.data()), fileSize);

    // Clean up temp file
    std::filesystem::remove(spirvPath);

    return CompileResult::ok(spirvCode);
}

bool ShaderCompiler::compileAndSave(const std::string& glslPath,
                                     const std::string& spirvPath,
                                     Shader::Stage stage) {
    if (!isCompilerAvailable()) {
        SPDLOG_ERROR("ShaderCompiler: glslangValidator not found");
        return false;
    }

    // Build command
    std::ostringstream cmd;
    cmd << s_glslangValidatorPath
        << " -V"                                    // Generate SPIR-V
        << " -o " << spirvPath                       // Output file
        << " " << glslPath;                          // Input file

    // Execute compilation
    FILE* pipe = popen(cmd.str().c_str(), "r");
    if (!pipe) {
        SPDLOG_ERROR("ShaderCompiler: Failed to execute glslangValidator");
        return false;
    }

    // Read any output (errors/warnings)
    char buffer[256];
    std::string output;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    int status = pclose(pipe);

    if (status != 0) {
        SPDLOG_ERROR("ShaderCompiler: Compilation failed for: {}", glslPath);
        if (!output.empty()) {
            SPDLOG_ERROR("ShaderCompiler: {}", output);
        }
        return false;
    }

    // Log warnings if any
    if (!output.empty()) {
        SPDLOG_WARN("ShaderCompiler: {}", output);
    }

    SPDLOG_INFO("ShaderCompiler: Compiled {} to {}", glslPath, spirvPath);
    return true;
}

std::string ShaderCompiler::getStageDefinition(Shader::Stage stage) {
    switch (stage) {
        case Shader::Stage::Vertex:
            return "#define VERTEX_STAGE\n";
        case Shader::Stage::Fragment:
            return "#define FRAGMENT_STAGE\n";
        case Shader::Stage::Geometry:
            return "#define GEOMETRY_STAGE\n";
        case Shader::Stage::TessControl:
            return "#define TESS_CONTROL_STAGE\n";
        case Shader::Stage::TessEvaluation:
            return "#define TESS_EVALUATION_STAGE\n";
        case Shader::Stage::Compute:
            return "#define COMPUTE_STAGE\n";
        default:
            return "";
    }
}
