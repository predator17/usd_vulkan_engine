#include "vulkan_swapchain.hpp"
#include "vulkan_context.hpp"

VulkanSwapchain::~VulkanSwapchain() {
    // Cleanup handled by explicit cleanup()
}

bool VulkanSwapchain::create(VulkanContext& context, vk::Extent2D extent) {
    if (m_initialized) {
        return false;
    }

    vk::Device device = context.getDevice();
    vk::PhysicalDevice physicalDevice = context.getPhysicalDevice();
    vk::SurfaceKHR surface = context.getSurface();

    // Choose surface format
    vk::SurfaceFormatKHR surfaceFormat;
    bool formatFound = false;
    std::vector<vk::SurfaceFormatKHR> formats = physicalDevice.getSurfaceFormatsKHR(surface);
    for (const auto& format : formats) {
        if (format.format == vk::Format::eB8G8R8A8Srgb) {
            surfaceFormat = format;
            formatFound = true;
            break;
        }
    }
    if (!formatFound) {
        surfaceFormat = formats[0];
    }
    m_imageFormat = surfaceFormat.format;

    // Choose present mode
    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
    std::vector<vk::PresentModeKHR> presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
    for (const auto& mode : presentModes) {
        if (mode == vk::PresentModeKHR::eMailbox) {
            presentMode = mode;
            break;
        }
    }

    // Choose extent
    vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    m_extent = extent;
    if (capabilities.currentExtent.width == 0xFFFFFFFF) {
        m_extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        m_extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    } else {
        m_extent = capabilities.currentExtent;
    }

    // Image count
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    // Create swapchain
    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.setSurface(surface);
    createInfo.setMinImageCount(imageCount);
    createInfo.setImageFormat(m_imageFormat);
    createInfo.setImageColorSpace(surfaceFormat.colorSpace);
    createInfo.setImageExtent(m_extent);
    createInfo.setImageArrayLayers(1);
    createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
    createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    createInfo.setPreTransform(capabilities.currentTransform);
    createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    createInfo.setPresentMode(presentMode);
    createInfo.setClipped(true);
    createInfo.setOldSwapchain(nullptr);

    vk::Result result = device.createSwapchainKHR(&createInfo, nullptr, &m_swapchain);
    if (result != vk::Result::eSuccess) {
        return false;
    }

    // Get images
    m_images = device.getSwapchainImagesKHR(m_swapchain);

    if (!createImageViews(context)) {
        cleanup(context);
        return false;
    }

    if (!createDepthBuffer(context)) {
        cleanup(context);
        return false;
    }

    if (!createRenderPass(context)) {
        cleanup(context);
        return false;
    }

    if (!createFramebuffers(context)) {
        cleanup(context);
        return false;
    }

    m_initialized = true;
    return true;
}

bool VulkanSwapchain::createImageViews(VulkanContext& context) {
    vk::Device device = context.getDevice();

    m_imageViews.resize(m_images.size());
    for (size_t i = 0; i < m_images.size(); ++i) {
        vk::ImageViewCreateInfo createInfo{};
        createInfo.setImage(m_images[i]);
        createInfo.setViewType(vk::ImageViewType::e2D);
        createInfo.setFormat(m_imageFormat);
        createInfo.setComponents(vk::ComponentMapping());
        createInfo.setSubresourceRange(vk::ImageSubresourceRange{
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
        });

        vk::Result result = device.createImageView(&createInfo, nullptr, &m_imageViews[i]);
        if (result != vk::Result::eSuccess) {
            return false;
        }
    }
    return true;
}

bool VulkanSwapchain::createDepthBuffer(VulkanContext& context) {
    vk::Device device = context.getDevice();
    m_depthFormat = vk::Format::eD24UnormS8Uint;

    // Create image
    vk::ImageCreateInfo imageInfo{};
    imageInfo.setImageType(vk::ImageType::e2D);
    imageInfo.setExtent(vk::Extent3D{m_extent.width, m_extent.height, 1});
    imageInfo.setMipLevels(1);
    imageInfo.setArrayLayers(1);
    imageInfo.setFormat(m_depthFormat);
    imageInfo.setTiling(vk::ImageTiling::eOptimal);
    imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
    imageInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);
    imageInfo.setSamples(vk::SampleCountFlagBits::e1);
    imageInfo.setSharingMode(vk::SharingMode::eExclusive);

    vk::Result result = device.createImage(&imageInfo, nullptr, &m_depthImage);
    if (result != vk::Result::eSuccess) {
        return false;
    }

    // Allocate memory
    vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(m_depthImage);
    vk::PhysicalDeviceMemoryProperties memProperties = context.getPhysicalDevice().getMemoryProperties();

    uint32_t memoryTypeIndex = 0;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal)) {
            memoryTypeIndex = i;
            break;
        }
    }

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memRequirements.size);
    allocInfo.setMemoryTypeIndex(memoryTypeIndex);

    result = device.allocateMemory(&allocInfo, nullptr, &m_depthMemory);
    if (result != vk::Result::eSuccess) {
        return false;
    }

    device.bindImageMemory(m_depthImage, m_depthMemory, 0);

    // Create image view
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.setImage(m_depthImage);
    viewInfo.setViewType(vk::ImageViewType::e2D);
    viewInfo.setFormat(m_depthFormat);
    viewInfo.setSubresourceRange(vk::ImageSubresourceRange{
        vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1
    });

    result = device.createImageView(&viewInfo, nullptr, &m_depthImageView);
    return result == vk::Result::eSuccess;
}

bool VulkanSwapchain::createRenderPass(VulkanContext& context) {
    vk::Device device = context.getDevice();

    // Color attachment
    vk::AttachmentDescription colorAttachment{};
    colorAttachment.setFormat(m_imageFormat);
    colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
    colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
    colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    // Depth attachment
    vk::AttachmentDescription depthAttachment{};
    depthAttachment.setFormat(m_depthFormat);
    depthAttachment.setSamples(vk::SampleCountFlagBits::e1);
    depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    depthAttachment.setStoreOp(vk::AttachmentStoreOp::eDontCare);
    depthAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    depthAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    depthAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
    depthAttachment.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::AttachmentReference colorRef{};
    colorRef.setAttachment(0);
    colorRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::AttachmentReference depthRef{};
    depthRef.setAttachment(1);
    depthRef.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::SubpassDescription subpass{};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subpass.setColorAttachmentCount(1);
    subpass.setPColorAttachments(&colorRef);
    subpass.setPDepthStencilAttachment(&depthRef);

    vk::SubpassDependency dependency{};
    dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
    dependency.setDstSubpass(0);
    dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    dependency.setSrcAccessMask(vk::AccessFlagBits::eNone);
    dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

    std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    vk::RenderPassCreateInfo createInfo{};
    createInfo.setAttachmentCount(attachments.size());
    createInfo.setPAttachments(attachments.data());
    createInfo.setSubpassCount(1);
    createInfo.setPSubpasses(&subpass);
    createInfo.setDependencyCount(1);
    createInfo.setPDependencies(&dependency);

    vk::Result result = device.createRenderPass(&createInfo, nullptr, &m_renderPass);
    return result == vk::Result::eSuccess;
}

bool VulkanSwapchain::createFramebuffers(VulkanContext& context) {
    vk::Device device = context.getDevice();
    m_framebuffers.resize(m_imageViews.size());

    for (size_t i = 0; i < m_imageViews.size(); ++i) {
        std::array<vk::ImageView, 2> attachments = {m_imageViews[i], m_depthImageView};

        vk::FramebufferCreateInfo createInfo{};
        createInfo.setRenderPass(m_renderPass);
        createInfo.setAttachmentCount(attachments.size());
        createInfo.setPAttachments(attachments.data());
        createInfo.setWidth(m_extent.width);
        createInfo.setHeight(m_extent.height);
        createInfo.setLayers(1);

        vk::Result result = device.createFramebuffer(&createInfo, nullptr, &m_framebuffers[i]);
        if (result != vk::Result::eSuccess) {
            return false;
        }
    }
    return true;
}

void VulkanSwapchain::cleanup(VulkanContext& context) {
    vk::Device device = context.getDevice();

    for (auto fb : m_framebuffers) {
        device.destroyFramebuffer(fb);
    }
    for (auto iv : m_imageViews) {
        device.destroyImageView(iv);
    }

    if (m_depthImageView) {
        device.destroyImageView(m_depthImageView);
    }
    if (m_depthImage) {
        device.destroyImage(m_depthImage);
    }
    if (m_depthMemory) {
        device.freeMemory(m_depthMemory);
    }

    if (m_swapchain) {
        device.destroySwapchainKHR(m_swapchain);
    }
    if (m_renderPass) {
        device.destroyRenderPass(m_renderPass);
    }

    m_initialized = false;
}

uint32_t VulkanSwapchain::acquireNextImage() {
    // This would require a semaphore - simplified for now
    return 0;
}
