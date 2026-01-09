#include "imgui_renderer.hpp"

#include "imgui.h"
#include "imgui_impl_vulkan.h"

#include "render/vulkan_context.hpp"
#include "render/vulkan_swapchain.hpp"

#include <spdlog/spdlog.h>
#include <glm/glm.hpp>

namespace usd_vk {

ImGuiRenderer::ImGuiRenderer() = default;

ImGuiRenderer::~ImGuiRenderer() {
    // Shutdown should be called explicitly
}

bool ImGuiRenderer::initialize(VulkanContext& context, vk::RenderPass renderPass) {
    if (m_initialized) {
        SPDLOG_WARN("ImGuiRenderer already initialized");
        return true;
    }

    vk::Device device = context.getDevice();
    vk::PhysicalDevice physicalDevice = context.getPhysicalDevice();
    vk::Instance instance = context.getInstance();
    vk::Queue queue = context.getGraphicsQueue();
    VulkanQueueFamilyIndices indices = context.getQueueFamilyIndices();
    VulkanSwapchain& swapchain = context.getSwapchain();

    m_imageCount = static_cast<uint32_t>(swapchain.getImageViews().size());

    // Create descriptor pool for ImGui
    if (!createDescriptorPool(device)) {
        SPDLOG_ERROR("Failed to create ImGui descriptor pool");
        return false;
    }

    // If no render pass provided, create one
    if (!renderPass) {
        if (!createRenderPass(context)) {
            SPDLOG_ERROR("Failed to create ImGui render pass");
            cleanup(context);
            return false;
        }
        renderPass = m_renderPass;
        m_ownsRenderPass = true;
    } else {
        m_renderPass = renderPass;
        m_ownsRenderPass = false;
    }

    // Initialize ImGui Vulkan backend
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.ApiVersion = VK_API_VERSION_1_3;
    initInfo.Instance = static_cast<VkInstance>(instance);
    initInfo.PhysicalDevice = static_cast<VkPhysicalDevice>(physicalDevice);
    initInfo.Device = static_cast<VkDevice>(device);
    initInfo.QueueFamily = indices.graphicsFamily.value();
    initInfo.Queue = static_cast<VkQueue>(queue);
    initInfo.DescriptorPool = static_cast<VkDescriptorPool>(m_descriptorPool);
    initInfo.MinImageCount = MIN_IMAGE_COUNT;
    initInfo.ImageCount = m_imageCount;
    initInfo.RenderPass = static_cast<VkRenderPass>(renderPass);
    initInfo.Subpass = 0;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.CheckVkResultFn = [](VkResult err) {
        if (err != VK_SUCCESS) {
            SPDLOG_ERROR("ImGui Vulkan error: {}", static_cast<int>(err));
        }
    };

    if (!ImGui_ImplVulkan_Init(&initInfo)) {
        SPDLOG_ERROR("Failed to initialize ImGui Vulkan backend");
        cleanup(context);
        return false;
    }

    // Note: Font texture is automatically uploaded on first frame in new ImGui versions

    // Create framebuffers if we own the render pass
    if (m_ownsRenderPass) {
        if (!createFrameBuffers(context)) {
            SPDLOG_ERROR("Failed to create ImGui framebuffers");
            cleanup(context);
            return false;
        }
    }

    m_initialized = true;
    SPDLOG_INFO("ImGuiRenderer initialized");
    return true;
}

void ImGuiRenderer::shutdown(VulkanContext& context) {
    if (!m_initialized) {
        return;
    }

    vk::Device device = context.getDevice();

    // Wait for device to be idle
    device.waitIdle();

    ImGui_ImplVulkan_Shutdown();

    cleanup(context);

    m_initialized = false;
    SPDLOG_INFO("ImGuiRenderer shutdown");
}

void ImGuiRenderer::cleanup(VulkanContext& context) {
    vk::Device device = context.getDevice();

    cleanupFrameBuffers(device);
    cleanupDescriptorPool(device);
    cleanupRenderPass(device);
}

bool ImGuiRenderer::createDescriptorPool(vk::Device device) {
    // ImGui requires a descriptor pool with combined image samplers
    std::array<vk::DescriptorPoolSize, 1> poolSizes = {
        vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1000)
    };

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    vk::Result result = device.createDescriptorPool(&poolInfo, nullptr, &m_descriptorPool);
    if (result != vk::Result::eSuccess) {
        SPDLOG_ERROR("Failed to create descriptor pool: {}", static_cast<int>(result));
        return false;
    }

    return true;
}

bool ImGuiRenderer::createRenderPass(VulkanContext& context) {
    vk::Device device = context.getDevice();
    VulkanSwapchain& swapchain = context.getSwapchain();
    vk::Format colorFormat = swapchain.getImageFormat();

    // Color attachment
    vk::AttachmentDescription colorAttachment = {};
    colorAttachment.format = colorFormat;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;

    vk::SubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                               vk::PipelineStageFlagBits::eEarlyFragmentTests;
    dependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite |
                                vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                               vk::PipelineStageFlagBits::eEarlyFragmentTests |
                               vk::PipelineStageFlagBits::eLateFragmentTests;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead |
                                vk::AccessFlagBits::eColorAttachmentWrite;

    vk::RenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    vk::Result result = device.createRenderPass(&renderPassInfo, nullptr, &m_renderPass);
    if (result != vk::Result::eSuccess) {
        SPDLOG_ERROR("Failed to create render pass: {}", static_cast<int>(result));
        return false;
    }

    return true;
}

bool ImGuiRenderer::createFrameBuffers(VulkanContext& context) {
    vk::Device device = context.getDevice();
    VulkanSwapchain& swapchain = context.getSwapchain();
    const std::vector<vk::ImageView>& imageViews = swapchain.getImageViews();
    vk::Extent2D extent = swapchain.getExtent();

    m_framebuffers.resize(imageViews.size());

    for (size_t i = 0; i < imageViews.size(); i++) {
        vk::FramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &imageViews[i];
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        vk::Result result = device.createFramebuffer(&framebufferInfo, nullptr, &m_framebuffers[i]);
        if (result != vk::Result::eSuccess) {
            SPDLOG_ERROR("Failed to create framebuffer {}: {}", i, static_cast<int>(result));
            return false;
        }
    }

    return true;
}

void ImGuiRenderer::cleanupRenderPass(vk::Device device) {
    if (m_renderPass && m_ownsRenderPass) {
        device.destroyRenderPass(m_renderPass);
        m_renderPass = nullptr;
    }
}

void ImGuiRenderer::cleanupFrameBuffers(vk::Device device) {
    for (auto& fb : m_framebuffers) {
        if (fb) {
            device.destroyFramebuffer(fb);
        }
    }
    m_framebuffers.clear();
}

void ImGuiRenderer::cleanupDescriptorPool(vk::Device device) {
    if (m_descriptorPool) {
        device.destroyDescriptorPool(m_descriptorPool);
        m_descriptorPool = nullptr;
    }
}

void ImGuiRenderer::newFrame() {
    if (!m_initialized) {
        return;
    }

    ImGui_ImplVulkan_NewFrame();
}

void ImGuiRenderer::renderDrawData(vk::CommandBuffer cmdBuffer) {
    if (!m_initialized) {
        return;
    }

    ImDrawData* drawData = ImGui::GetDrawData();
    if (drawData && drawData->DisplaySize.x > 0 && drawData->DisplaySize.y > 0) {
        ImGui_ImplVulkan_RenderDrawData(drawData, cmdBuffer);
    }
}

} // namespace usd_vk
