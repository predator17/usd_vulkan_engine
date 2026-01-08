#include "renderer.hpp"
#include "vulkan_context.hpp"
#include "vulkan_swapchain.hpp"
#include "vulkan_pipeline.hpp"
#include "shader.hpp"
#include "scene/scene.hpp"
#include "camera/camera.hpp"
#include "ui/imgui_renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <spdlog/spdlog.h>

// Vertex structure for Vulkan
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 normal;

    static vk::VertexInputBindingDescription getBindingDescription() {
        return vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex);
    }

    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 3> attributes;

        // Position (location 0)
        attributes[0].setBinding(0);
        attributes[0].setLocation(0);
        attributes[0].setFormat(vk::Format::eR32G32B32Sfloat);
        attributes[0].setOffset(offsetof(Vertex, pos));

        // Color (location 1)
        attributes[1].setBinding(0);
        attributes[1].setLocation(1);
        attributes[1].setFormat(vk::Format::eR32G32B32Sfloat);
        attributes[1].setOffset(offsetof(Vertex, color));

        // Normal (location 2)
        attributes[2].setBinding(0);
        attributes[2].setLocation(2);
        attributes[2].setFormat(vk::Format::eR32G32B32Sfloat);
        attributes[2].setOffset(offsetof(Vertex, normal));

        return attributes;
    }
};

Renderer::Renderer() = default;

Renderer::~Renderer() {
    // Cleanup should be done via cleanup()
}

Renderer::Renderer(Renderer&& other) noexcept
    : m_renderPass(std::exchange(other.m_renderPass, vk::RenderPass()))
    , m_framebuffers(std::move(other.m_framebuffers))
    , m_commandBuffers(std::move(other.m_commandBuffers))
    , m_renderFinished(std::move(other.m_renderFinished))
    , m_imageAvailable(std::move(other.m_imageAvailable))
    , m_inFlightFences(std::move(other.m_inFlightFences))
    , m_vertexShader(std::move(other.m_vertexShader))
    , m_fragmentShader(std::move(other.m_fragmentShader))
    , m_pipeline(std::move(other.m_pipeline))
    , m_meshBuffers(std::move(other.m_meshBuffers))
    , m_scene(other.m_scene)
    , m_currentFrame(other.m_currentFrame)
    , m_initialized(other.m_initialized)
    , m_framebufferResized(other.m_framebufferResized)
{
    other.m_scene = nullptr;
    other.m_currentFrame = 0;
    other.m_initialized = false;
}

Renderer& Renderer::operator=(Renderer&& other) noexcept {
    if (this != &other) {
        // Note: proper cleanup requires VulkanContext, skip for now
        // In production, you'd need to store the context or pass it in

        m_renderPass = std::exchange(other.m_renderPass, vk::RenderPass());
        m_framebuffers = std::move(other.m_framebuffers);
        m_commandBuffers = std::move(other.m_commandBuffers);
        m_renderFinished = std::move(other.m_renderFinished);
        m_imageAvailable = std::move(other.m_imageAvailable);
        m_inFlightFences = std::move(other.m_inFlightFences);
        m_vertexShader = std::move(other.m_vertexShader);
        m_fragmentShader = std::move(other.m_fragmentShader);
        m_pipeline = std::move(other.m_pipeline);
        m_meshBuffers = std::move(other.m_meshBuffers);
        m_scene = other.m_scene;
        m_currentFrame = other.m_currentFrame;
        m_initialized = other.m_initialized;
        m_framebufferResized = other.m_framebufferResized;

        other.m_scene = nullptr;
        other.m_currentFrame = 0;
        other.m_initialized = false;
    }
    return *this;
}

bool Renderer::initialize(VulkanContext& context) {
    if (!createRenderPass(context)) {
        SPDLOG_ERROR("Failed to create render pass");
        return false;
    }

    if (!createPipeline(context)) {
        SPDLOG_ERROR("Failed to create pipeline");
        return false;
    }

    if (!createFramebuffers(context)) {
        SPDLOG_ERROR("Failed to create framebuffers");
        return false;
    }

    if (!createCommandBuffers(context)) {
        SPDLOG_ERROR("Failed to create command buffers");
        return false;
    }

    if (!createSyncObjects(context)) {
        SPDLOG_ERROR("Failed to create sync objects");
        return false;
    }

    m_initialized = true;
    SPDLOG_INFO("Renderer initialized");
    return true;
}

void Renderer::setScene(Scene* scene) {
    m_scene = scene;
}

void Renderer::render(VulkanContext& context, Scene* scene) {
    if (scene) {
        m_scene = scene;
    }

    if (!m_initialized || !m_scene) {
        return;
    }

    // Create mesh buffers if not exists (lazy initialization)
    if (m_meshBuffers.empty() && m_scene->getMeshCount() > 0) {
        createMeshBuffers(context, m_scene);
    }

    vk::Device device = context.getDevice();
    VulkanSwapchain& swapchain = context.getSwapchain();

    // Check if swapchain needs recreation
    if (context.getFramebufferResized()) {
        context.setFramebufferResized(false);
        // Would need to recreate swapchain here
        return;
    }

    // Wait for previous frame
    device.waitForFences(1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    // Acquire image from swapchain
    uint32_t imageIndex;
    vk::Result result = device.acquireNextImageKHR(
        swapchain.getSwapchain(),
        UINT64_MAX,
        m_imageAvailable[m_currentFrame],
        vk::Fence(),
        &imageIndex
    );

    if (result == vk::Result::eErrorOutOfDateKHR) {
        // Swapchain needs recreation
        return;
    } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        SPDLOG_ERROR("Failed to acquire swapchain image");
        return;
    }

    // Reset fence for this frame (must be before submission)
    if (device.resetFences(1, &m_inFlightFences[m_currentFrame]) != vk::Result::eSuccess) {
        SPDLOG_ERROR("Failed to reset fence");
        return;
    }

    // Update scene animation
    m_scene->updateAnimation(context.getElapsedTime());

    // Re-record command buffer with updated push constants
    {
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        m_commandBuffers[imageIndex].reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        m_commandBuffers[imageIndex].begin(beginInfo);

        vk::RenderPassBeginInfo renderPassInfo;
        renderPassInfo.setRenderPass(m_renderPass);
        renderPassInfo.setFramebuffer(m_framebuffers[imageIndex]);
        renderPassInfo.setRenderArea({ {0, 0}, swapchain.getExtent() });

        // Clear values for color and depth
        std::array<vk::ClearValue, 2> clearValues{};
        clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f });
        clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
        renderPassInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()));
        renderPassInfo.setPClearValues(clearValues.data());

        m_commandBuffers[imageIndex].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

        // Select pipeline based on wireframe mode
        VulkanPipeline* activePipeline = m_wireframeMode ? m_wireframePipeline.get() : m_pipeline.get();
        if (activePipeline && activePipeline->getPipeline()) {
            m_commandBuffers[imageIndex].bindPipeline(
                vk::PipelineBindPoint::eGraphics,
                activePipeline->getPipeline()
            );

            // Create viewport and scissor
            vk::Viewport viewport;
            viewport.setX(0.0f);
            viewport.setY(0.0f);
            viewport.setWidth(static_cast<float>(swapchain.getExtent().width));
            viewport.setHeight(static_cast<float>(swapchain.getExtent().height));
            viewport.setMinDepth(0.0f);
            viewport.setMaxDepth(1.0f);

            vk::Rect2D scissor;
            scissor.setOffset({ 0, 0 });
            scissor.setExtent(swapchain.getExtent());

            m_commandBuffers[imageIndex].setViewport(0, 1, &viewport);
            m_commandBuffers[imageIndex].setScissor(0, 1, &scissor);

            // Bind vertex and index buffers
            if (!m_meshBuffers.empty() && m_meshBuffers[0].valid) {
                vk::Buffer vertexBuffers[] = { m_meshBuffers[0].vertexBuffer };
                vk::DeviceSize offsets[] = { 0 };
                m_commandBuffers[imageIndex].bindVertexBuffers(0, 1, vertexBuffers, offsets);
                m_commandBuffers[imageIndex].bindIndexBuffer(m_meshBuffers[0].indexBuffer, 0, vk::IndexType::eUint32);

                // Push constants with updated transform
                updateAndPushConstants(context, m_commandBuffers[imageIndex]);

                // Draw
                m_commandBuffers[imageIndex].drawIndexed(m_meshBuffers[0].indexCount, 1, 0, 0, 0);
            }
        }

        // Render ImGui UI overlay
        if (m_imguiRenderer) {
            m_imguiRenderer->renderDrawData(m_commandBuffers[imageIndex]);
        }

        m_commandBuffers[imageIndex].endRenderPass();
        m_commandBuffers[imageIndex].end();
    }

    // Submit command buffer
    vk::SubmitInfo submitInfo;
    vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    submitInfo.setWaitSemaphoreCount(1);
    submitInfo.setPWaitSemaphores(&m_imageAvailable[m_currentFrame]);
    submitInfo.setPWaitDstStageMask(&waitStages);
    submitInfo.setCommandBufferCount(1);
    submitInfo.setPCommandBuffers(&m_commandBuffers[imageIndex]);
    submitInfo.setSignalSemaphoreCount(1);
    submitInfo.setPSignalSemaphores(&m_renderFinished[m_currentFrame]);

    result = context.getGraphicsQueue().submit(1, &submitInfo, m_inFlightFences[m_currentFrame]);
    if (result != vk::Result::eSuccess) {
        SPDLOG_ERROR("Failed to submit draw command buffer");
        return;
    }

    // Present
    vk::SwapchainKHR swapchainHandle = swapchain.getSwapchain();
    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphoreCount(1);
    presentInfo.setPWaitSemaphores(&m_renderFinished[m_currentFrame]);
    presentInfo.setSwapchainCount(1);
    presentInfo.setPSwapchains(&swapchainHandle);
    presentInfo.setPImageIndices(&imageIndex);

    result = context.getPresentQueue().presentKHR(&presentInfo);

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
        // Swapchain needs recreation
    } else if (result != vk::Result::eSuccess) {
        SPDLOG_ERROR("Failed to present swapchain image");
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::recordCommandBuffers(VulkanContext& context) {
    // Re-record command buffers when scene changes
    vk::Device device = context.getDevice();

    for (size_t i = 0; i < m_commandBuffers.size(); i++) {
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

        m_commandBuffers[i].reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        m_commandBuffers[i].begin(beginInfo);

        VulkanSwapchain& swapchain = context.getSwapchain();

        vk::RenderPassBeginInfo renderPassInfo;
        renderPassInfo.setRenderPass(m_renderPass);
        renderPassInfo.setFramebuffer(m_framebuffers[i]);
        renderPassInfo.setRenderArea({ {0, 0}, swapchain.getExtent() });

        vk::ClearValue clearColor(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f });
        renderPassInfo.setClearValueCount(1);
        renderPassInfo.setPClearValues(&clearColor);

        m_commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

        // Select pipeline based on wireframe mode
        VulkanPipeline* activePipeline = m_wireframeMode ? m_wireframePipeline.get() : m_pipeline.get();
        if (activePipeline && activePipeline->getPipeline()) {
            m_commandBuffers[i].bindPipeline(
                vk::PipelineBindPoint::eGraphics,
                activePipeline->getPipeline()
            );

            // Create viewport and scissor
            vk::Viewport viewport;
            viewport.setX(0.0f);
            viewport.setY(0.0f);
            viewport.setWidth(static_cast<float>(swapchain.getExtent().width));
            viewport.setHeight(static_cast<float>(swapchain.getExtent().height));
            viewport.setMinDepth(0.0f);
            viewport.setMaxDepth(1.0f);

            vk::Rect2D scissor;
            scissor.setOffset({ 0, 0 });
            scissor.setExtent(swapchain.getExtent());

            m_commandBuffers[i].setViewport(0, 1, &viewport);
            m_commandBuffers[i].setScissor(0, 1, &scissor);

            // Bind vertex and index buffers
            if (!m_meshBuffers.empty() && m_meshBuffers[0].valid) {
                vk::Buffer vertexBuffers[] = { m_meshBuffers[0].vertexBuffer };
                vk::DeviceSize offsets[] = { 0 };
                m_commandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);
                m_commandBuffers[i].bindIndexBuffer(m_meshBuffers[0].indexBuffer, 0, vk::IndexType::eUint32);

                // Draw
                m_commandBuffers[i].drawIndexed(m_meshBuffers[0].indexCount, 1, 0, 0, 0);
            }
        }

        m_commandBuffers[i].endRenderPass();
        m_commandBuffers[i].end();
    }
}

void Renderer::cleanup(VulkanContext& context) {
    vk::Device device = context.getDevice();

    device.waitIdle();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        device.destroyFence(m_inFlightFences[i]);
        device.destroySemaphore(m_renderFinished[i]);
        device.destroySemaphore(m_imageAvailable[i]);
    }

    // Framebuffers and render pass are owned by swapchain, don't destroy them here
    m_framebuffers.clear();

    for (auto& meshBuf : m_meshBuffers) {
        if (meshBuf.valid) {
            device.destroyBuffer(meshBuf.vertexBuffer);
            device.freeMemory(meshBuf.vertexBufferMemory);
            device.destroyBuffer(meshBuf.indexBuffer);
            device.freeMemory(meshBuf.indexBufferMemory);
        }
    }

    // Render pass is owned by swapchain, don't destroy it
    m_renderPass = vk::RenderPass();

    m_pipeline.reset();
    m_wireframePipeline.reset();
    m_vertexShader.reset();
    m_fragmentShader.reset();

    m_initialized = false;
}

bool Renderer::createRenderPass(VulkanContext& context) {
    // Use swapchain's render pass directly
    VulkanSwapchain& swapchain = context.getSwapchain();
    m_renderPass = swapchain.getRenderPass();
    return m_renderPass ? true : false;
}

bool Renderer::createFramebuffers(VulkanContext& context) {
    // Use swapchain's framebuffers directly
    VulkanSwapchain& swapchain = context.getSwapchain();
    m_framebuffers = swapchain.getFramebuffers();
    return !m_framebuffers.empty();
}

bool Renderer::createCommandBuffers(VulkanContext& context) {
    vk::Device device = context.getDevice();
    VulkanSwapchain& swapchain = context.getSwapchain();

    m_commandBuffers.resize(swapchain.getImageViews().size());

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.setCommandPool(context.getCommandPool());
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocInfo.setCommandBufferCount(static_cast<uint32_t>(m_commandBuffers.size()));

    vk::Result result = device.allocateCommandBuffers(&allocInfo, m_commandBuffers.data());
    if (result != vk::Result::eSuccess) {
        SPDLOG_ERROR("Failed to allocate command buffers");
        return false;
    }

    // Command buffers will be recorded when mesh data is available (in render())
    SPDLOG_INFO("Command buffers allocated: {}", m_commandBuffers.size());
    return true;
}

bool Renderer::createSyncObjects(VulkanContext& context) {
    vk::Device device = context.getDevice();

    m_imageAvailable.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinished.resize(MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    vk::SemaphoreCreateInfo semaphoreInfo;
    vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::Result result1 = device.createSemaphore(&semaphoreInfo, nullptr, &m_imageAvailable[i]);
        vk::Result result2 = device.createSemaphore(&semaphoreInfo, nullptr, &m_renderFinished[i]);
        vk::Result result3 = device.createFence(&fenceInfo, nullptr, &m_inFlightFences[i]);

        if (result1 != vk::Result::eSuccess || result2 != vk::Result::eSuccess || result3 != vk::Result::eSuccess) {
            SPDLOG_ERROR("Failed to create sync objects");
            return false;
        }
    }

    return true;
}

bool Renderer::createPipeline(VulkanContext& context) {
    m_vertexShader = std::make_unique<Shader>();
    m_fragmentShader = std::make_unique<Shader>();

    // Load shaders
    if (!m_vertexShader->loadFromSPIRV(context, "shaders/basic.vert.spv", Shader::Stage::Vertex)) {
        SPDLOG_ERROR("Failed to load vertex shader");
        return false;
    }

    if (!m_fragmentShader->loadFromSPIRV(context, "shaders/basic.frag.spv", Shader::Stage::Fragment)) {
        SPDLOG_ERROR("Failed to load fragment shader");
        return false;
    }

    // Create fill mode pipeline (solid rendering)
    m_pipeline = std::make_unique<VulkanPipeline>();

    // Attach shaders to fill pipeline
    if (!m_pipeline->attachVertexShader(context, *m_vertexShader)) {
        SPDLOG_ERROR("Failed to attach vertex shader");
        return false;
    }

    if (!m_pipeline->attachFragmentShader(context, *m_fragmentShader)) {
        SPDLOG_ERROR("Failed to attach fragment shader");
        return false;
    }

    // Create pipeline layout with push constant range
    vk::PushConstantRange pushConstantRange;
    pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex);
    pushConstantRange.setOffset(0);
    pushConstantRange.setSize(sizeof(UniformBufferObject));

    if (!m_pipeline->createPipelineLayout(context, { pushConstantRange })) {
        SPDLOG_ERROR("Failed to create pipeline layout");
        return false;
    }

    // Create graphics pipeline config
    PipelineConfig config;
    config.renderPass = m_renderPass;
    config.extent = context.getSwapchain().getExtent();
    config.vertexBinding = Vertex::getBindingDescription();
    auto attribs = Vertex::getAttributeDescriptions();
    config.vertexAttributes.assign(attribs.begin(), attribs.end());
    config.polygonMode = vk::PolygonMode::eFill;  // Solid fill

    if (!m_pipeline->createGraphicsPipeline(context, config)) {
        SPDLOG_ERROR("Failed to create graphics pipeline");
        return false;
    }

    // Create wireframe pipeline (shares shaders and layout)
    m_wireframePipeline = std::make_unique<VulkanPipeline>();

    // Attach same shaders to wireframe pipeline
    if (!m_wireframePipeline->attachVertexShader(context, *m_vertexShader)) {
        SPDLOG_ERROR("Failed to attach vertex shader to wireframe pipeline");
        return false;
    }

    if (!m_wireframePipeline->attachFragmentShader(context, *m_fragmentShader)) {
        SPDLOG_ERROR("Failed to attach fragment shader to wireframe pipeline");
        return false;
    }

    // Share the same pipeline layout
    if (!m_wireframePipeline->createPipelineLayout(context, { pushConstantRange })) {
        SPDLOG_ERROR("Failed to create wireframe pipeline layout");
        return false;
    }

    // Create wireframe graphics pipeline (only difference is polygon mode)
    config.polygonMode = vk::PolygonMode::eLine;  // Wireframe

    if (!m_wireframePipeline->createGraphicsPipeline(context, config)) {
        SPDLOG_ERROR("Failed to create wireframe graphics pipeline");
        return false;
    }

    SPDLOG_INFO("Created fill and wireframe pipelines");
    return true;
}

void Renderer::updateAndPushConstants(VulkanContext& context, vk::CommandBuffer cmdBuffer) {
    // Calculate UBO
    UniformBufferObject ubo;

    // Model matrix - with rotation from scene
    if (m_scene && !m_scene->getMeshes().empty()) {
        const auto& mesh = m_scene->getMeshes()[0];

        // Create transform matrix
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), mesh.translation);
        glm::mat4 rotation = glm::mat4_cast(mesh.rotation);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), mesh.scale);
        ubo.model = translation * rotation * scale;
    } else {
        ubo.model = glm::mat4(1.0f);
    }

    // View matrix - from camera or default
    if (m_camera) {
        ubo.view = m_camera->getViewMatrix();
    } else {
        ubo.view = glm::lookAt(
            glm::vec3(2.0f, 2.0f, 2.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
    }

    // Projection matrix - from camera or default
    auto extent = context.getSwapchain().getExtent();
    if (m_camera) {
        ubo.proj = m_camera->getProjectionMatrix(
            static_cast<float>(extent.width) / static_cast<float>(extent.height)
        );
    } else {
        ubo.proj = glm::perspective(
            glm::radians(45.0f),
            static_cast<float>(extent.width) / static_cast<float>(extent.height),
            0.1f,
            100.0f
        );
    }
    ubo.proj[1][1] *= -1;  // Flip Y for Vulkan

    // Time for animated noise displacement
    ubo.time = context.getElapsedTime();

    // Push constants (use active pipeline's layout)
    VulkanPipeline* activePipeline = m_wireframeMode ? m_wireframePipeline.get() : m_pipeline.get();
    cmdBuffer.pushConstants(
        activePipeline->getPipelineLayout(),
        vk::ShaderStageFlagBits::eVertex,
        0,
        sizeof(UniformBufferObject),
        &ubo
    );
}

bool Renderer::createMeshBuffers(VulkanContext& context, Scene* scene) {
    if (!scene || scene->getMeshes().empty()) {
        SPDLOG_WARN("No scene or meshes to create buffers for");
        return true;
    }

    vk::Device device = context.getDevice();

    // Clear old buffers
    for (auto& meshBuf : m_meshBuffers) {
        if (meshBuf.valid) {
            device.destroyBuffer(meshBuf.vertexBuffer);
            device.freeMemory(meshBuf.vertexBufferMemory);
            device.destroyBuffer(meshBuf.indexBuffer);
            device.freeMemory(meshBuf.indexBufferMemory);
        }
    }
    m_meshBuffers.clear();

    m_meshBuffers.resize(scene->getMeshCount());

    for (size_t meshIdx = 0; meshIdx < scene->getMeshCount(); meshIdx++) {
        const auto& meshData = scene->getMeshes()[meshIdx];

        // Create vertex buffer
        std::vector<Vertex> vertices;
        vertices.reserve(meshData.vertices.size());

        for (size_t i = 0; i < meshData.vertices.size(); i++) {
            Vertex v;
            v.pos = meshData.vertices[i];
            v.color = i < meshData.colors.size() ? meshData.colors[i] : meshData.baseColor;
            v.normal = i < meshData.normals.size() ? meshData.normals[i] : glm::vec3(0, 0, 1);
            vertices.push_back(v);
        }

        vk::DeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();

        vk::BufferCreateInfo vertexBufferInfo;
        vertexBufferInfo.setSize(vertexBufferSize);
        vertexBufferInfo.setUsage(vk::BufferUsageFlagBits::eVertexBuffer);
        vertexBufferInfo.setSharingMode(vk::SharingMode::eExclusive);

        vk::Result vbufResult = device.createBuffer(&vertexBufferInfo, nullptr, &m_meshBuffers[meshIdx].vertexBuffer);

        vk::MemoryRequirements vertexMemReqs = device.getBufferMemoryRequirements(m_meshBuffers[meshIdx].vertexBuffer);
        vk::MemoryAllocateInfo vertexAllocInfo;
        vertexAllocInfo.setAllocationSize(vertexMemReqs.size);
        vertexAllocInfo.setMemoryTypeIndex(
            context.findMemoryType(vertexMemReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible)
        );

        vk::Result vmemResult = device.allocateMemory(&vertexAllocInfo, nullptr, &m_meshBuffers[meshIdx].vertexBufferMemory);
        device.bindBufferMemory(m_meshBuffers[meshIdx].vertexBuffer, m_meshBuffers[meshIdx].vertexBufferMemory, 0);

        // Copy vertex data
        void* vertexData;
        device.mapMemory(m_meshBuffers[meshIdx].vertexBufferMemory, 0, vertexBufferSize, {}, &vertexData);
        memcpy(vertexData, vertices.data(), vertexBufferSize);
        device.unmapMemory(m_meshBuffers[meshIdx].vertexBufferMemory);

        // Create index buffer
        vk::DeviceSize indexBufferSize = sizeof(meshData.indices[0]) * meshData.indices.size();

        vk::BufferCreateInfo indexBufferInfo;
        indexBufferInfo.setSize(indexBufferSize);
        indexBufferInfo.setUsage(vk::BufferUsageFlagBits::eIndexBuffer);
        indexBufferInfo.setSharingMode(vk::SharingMode::eExclusive);

        vk::Result ibufResult = device.createBuffer(&indexBufferInfo, nullptr, &m_meshBuffers[meshIdx].indexBuffer);

        vk::MemoryRequirements indexMemReqs = device.getBufferMemoryRequirements(m_meshBuffers[meshIdx].indexBuffer);
        vk::MemoryAllocateInfo indexAllocInfo;
        indexAllocInfo.setAllocationSize(indexMemReqs.size);
        indexAllocInfo.setMemoryTypeIndex(
            context.findMemoryType(indexMemReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible)
        );

        vk::Result imemResult = device.allocateMemory(&indexAllocInfo, nullptr, &m_meshBuffers[meshIdx].indexBufferMemory);
        device.bindBufferMemory(m_meshBuffers[meshIdx].indexBuffer, m_meshBuffers[meshIdx].indexBufferMemory, 0);

        // Copy index data
        void* indexData;
        device.mapMemory(m_meshBuffers[meshIdx].indexBufferMemory, 0, indexBufferSize, {}, &indexData);
        memcpy(indexData, meshData.indices.data(), indexBufferSize);
        device.unmapMemory(m_meshBuffers[meshIdx].indexBufferMemory);

        m_meshBuffers[meshIdx].indexCount = static_cast<uint32_t>(meshData.indices.size());
        m_meshBuffers[meshIdx].valid = true;
    }

    return true;
}
