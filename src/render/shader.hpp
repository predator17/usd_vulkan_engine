#pragma once

#include <vulkan/vulkan.hpp>
#include <string>
#include <filesystem>
#include <vector>

class VulkanContext;

/**
 * @brief Shader class for managing Vulkan shader modules with hot-reload support
 *
 * Supports loading from pre-compiled SPIR-V files or compiling from GLSL source.
 * Tracks file modification time for hot-reload functionality.
 */
class Shader {
public:
    /**
     * @brief Shader stage types
     */
    enum class Stage {
        Vertex,
        Fragment,
        Geometry,
        TessControl,
        TessEvaluation,
        Compute
    };

    Shader() = default;
    ~Shader();

    // Non-copyable
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Movable
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    /**
     * @brief Load shader from pre-compiled SPIR-V file
     * @param context Vulkan context
     * @param path Path to .spv file
     * @param stage Shader stage
     * @return true if successful
     */
    bool loadFromSPIRV(VulkanContext& context, const std::string& path, Stage stage);

    /**
     * @brief Load and compile shader from GLSL source file
     * @param context Vulkan context
     * @param path Path to .vert, .frag, .geom, .comp, .tesc, or .tese file
     * @param stage Shader stage
     * @return true if successful
     */
    bool loadFromGLSL(VulkanContext& context, const std::string& path, Stage stage);

    /**
     * @brief Check if shader source file has been modified and reload if needed
     * @param context Vulkan context
     * @return true if shader was reloaded
     */
    bool checkForReload(VulkanContext& context);

    /**
     * @brief Cleanup shader resources
     * @param context Vulkan context
     */
    void cleanup(VulkanContext& context);

    // Getters
    vk::ShaderModule getModule() const { return m_shaderModule; }
    Stage getStage() const { return m_stage; }
    const std::string& getPath() const { return m_path; }
    bool isInitialized() const { return m_initialized; }
    bool isGLSL() const { return m_isGLSL; }

    /**
     * @brief Convert Stage to vk::ShaderStageFlagBits
     */
    vk::ShaderStageFlagBits getVkStage() const;

    /**
     * @brief Get file extension for a stage
     */
    static const char* getStageExtension(Stage stage);

    /**
     * @brief Get stage name string
     */
    static const char* getStageName(Stage stage);

private:
    bool createShaderModule(VulkanContext& context, const std::vector<uint32_t>& spirvCode);
    bool loadSPIRVFile(const std::string& path, std::vector<uint32_t>& outCode);
    void updateLastWriteTime();

    vk::ShaderModule m_shaderModule;
    Stage m_stage = Stage::Vertex;
    std::string m_path;
    std::filesystem::file_time_type m_lastWriteTime;
    bool m_initialized = false;
    bool m_isGLSL = false;
};
