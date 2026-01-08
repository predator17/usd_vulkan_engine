#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

class VulkanContext;
class Scene;
class Shader;
class VulkanPipeline;
class VulkanBuffer;
class Camera;

namespace usd_vk {
    class ImGuiRenderer;
}

/**
 * @brief Main renderer for USD Vulkan Engine
 *
 * Handles command buffer recording, synchronization, and drawing.
 */
class Renderer {
public:
    Renderer();
    ~Renderer();

    // Non-copyable
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    // Movable
    Renderer(Renderer&& other) noexcept;
    Renderer& operator=(Renderer&& other) noexcept;

    /**
     * @brief Initialize the renderer
     * @param context Vulkan context
     * @return true if successful
     */
    bool initialize(VulkanContext& context);

    /**
     * @brief Set the current scene
     */
    void setScene(Scene* scene);

    /**
     * @brief Set the ImGui renderer for UI overlay
     */
    void setImGuiRenderer(usd_vk::ImGuiRenderer* imguiRenderer) { m_imguiRenderer = imguiRenderer; }

    /**
     * @brief Set the camera for view/projection matrices
     */
    void setCamera(Camera* camera) { m_camera = camera; }

    /**
     * @brief Set wireframe display mode
     */
    void setWireframeMode(bool wireframe) { m_wireframeMode = wireframe; }

    /**
     * @brief Check if wireframe mode is enabled
     */
    bool isWireframeMode() const { return m_wireframeMode; }

    /**
     * @brief Render a frame
     * @param context Vulkan context
     * @param scene Scene to render (can be null to use previously set scene)
     */
    void render(VulkanContext& context, Scene* scene = nullptr);

    /**
     * @brief Record command buffers for current frame
     */
    void recordCommandBuffers(VulkanContext& context);

    /**
     * @brief Get the current frame index
     */
    uint32_t getCurrentFrame() const { return m_currentFrame; }

    /**
     * @brief Check if renderer is initialized
     */
    bool isInitialized() const { return m_initialized; }

    /**
     * @brief Cleanup resources
     */
    void cleanup(VulkanContext& context);

private:
    bool createRenderPass(VulkanContext& context);
    bool createFramebuffers(VulkanContext& context);
    bool createCommandBuffers(VulkanContext& context);
    bool createSyncObjects(VulkanContext& context);
    bool createPipeline(VulkanContext& context);

    void updateAndPushConstants(VulkanContext& context, vk::CommandBuffer cmdBuffer);

    bool createMeshBuffers(VulkanContext& context, Scene* scene);

    // Vulkan objects
    vk::RenderPass m_renderPass;
    std::vector<vk::Framebuffer> m_framebuffers;
    std::vector<vk::CommandBuffer> m_commandBuffers;

    // Synchronization
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    std::vector<vk::Semaphore> m_renderFinished;
    std::vector<vk::Semaphore> m_imageAvailable;
    std::vector<vk::Fence> m_inFlightFences;

    // Pipeline and shaders
    std::unique_ptr<Shader> m_vertexShader;
    std::unique_ptr<Shader> m_fragmentShader;
    std::unique_ptr<VulkanPipeline> m_pipeline;      // Fill mode pipeline
    std::unique_ptr<VulkanPipeline> m_wireframePipeline;  // Wireframe mode pipeline

    // Mesh buffers
    struct MeshBuffers {
        vk::Buffer vertexBuffer;
        vk::DeviceMemory vertexBufferMemory;
        vk::Buffer indexBuffer;
        vk::DeviceMemory indexBufferMemory;
        uint32_t indexCount;
        bool valid = false;
    };
    std::vector<MeshBuffers> m_meshBuffers;

    // Push constant data (same as UBO)
    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        float time;
    };

    Scene* m_scene = nullptr;
    uint32_t m_currentFrame = 0;
    bool m_initialized = false;
    bool m_framebufferResized = false;

    // UI overlay
    usd_vk::ImGuiRenderer* m_imguiRenderer = nullptr;

    // Camera
    Camera* m_camera = nullptr;

    // Display mode
    bool m_wireframeMode = false;
};
