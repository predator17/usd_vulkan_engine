#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>

class VulkanContext;

namespace usd_vk {

/**
 * @brief Vulkan renderer for ImGui
 *
 * Handles the Vulkan-specific rendering for Dear ImGui including:
 * - Descriptor pool for ImGui
 * - Render pass (uses existing or creates subpass)
 * - Font texture upload
 * - Command buffer recording for ImGui draw data
 */
class ImGuiRenderer {
public:
    ImGuiRenderer();
    ~ImGuiRenderer();

    // Non-copyable
    ImGuiRenderer(const ImGuiRenderer&) = delete;
    ImGuiRenderer& operator=(const ImGuiRenderer&) = delete;

    /**
     * @brief Initialize ImGui Vulkan renderer
     * @param context Vulkan context
     * @param renderPass Optional existing render pass (if null, creates one)
     * @return true if successful
     */
    bool initialize(VulkanContext& context, vk::RenderPass renderPass = nullptr);

    /**
     * @brief Shutdown renderer
     */
    void shutdown(VulkanContext& context);

    /**
     * @brief Check if initialized
     */
    bool isInitialized() const { return m_initialized; }

    /**
     * @brief New frame - call before creating UI widgets
     */
    void newFrame();

    /**
     * @brief Render ImGui draw data to command buffer
     * @param cmdBuffer Command buffer to record into
     */
    void renderDrawData(vk::CommandBuffer cmdBuffer);

    /**
     * @brief Get or create render pass for ImGui
     */
    vk::RenderPass getRenderPass() const { return m_renderPass; }

    /**
     * @brief Check if we own the render pass (created by us)
     */
    bool ownsRenderPass() const { return m_ownsRenderPass; }

private:
    bool createDescriptorPool(vk::Device device);
    bool createRenderPass(VulkanContext& context);
    bool createFrameBuffers(VulkanContext& context);

    void cleanup(VulkanContext& context);
    void cleanupRenderPass(vk::Device device);
    void cleanupFrameBuffers(vk::Device device);
    void cleanupDescriptorPool(vk::Device device);

    bool m_initialized = false;
    bool m_ownsRenderPass = false;

    vk::DescriptorPool m_descriptorPool;
    vk::RenderPass m_renderPass;
    vk::Pipeline m_pipeline;
    vk::PipelineLayout m_pipelineLayout;

    // Font texture
    vk::Image m_fontImage;
    vk::DeviceMemory m_fontImageMemory;
    vk::ImageView m_fontImageView;
    vk::Sampler m_fontSampler;
    vk::DescriptorSet m_fontDescriptorSet;

    // Frame data
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    std::vector<vk::Framebuffer> m_framebuffers;
    std::vector<vk::CommandBuffer> m_commandBuffers;

    uint32_t m_frameIndex = 0;
    uint32_t m_imageCount = 0;

    // Init info for ImGui
    static constexpr uint32_t MIN_IMAGE_COUNT = 2;
};

} // namespace usd_vk
