#include "vulkan_pipeline.hpp"
#include "vulkan_context.hpp"

VulkanPipeline::~VulkanPipeline() {
}

bool VulkanPipeline::createVertexShaderModule(VulkanContext& context, const std::vector<uint32_t>& spirvCode) {
    if (spirvCode.empty() || m_hasVertexShader) return false;

    vk::Device device = context.getDevice();
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.setCodeSize(spirvCode.size() * sizeof(uint32_t));
    createInfo.setPCode(spirvCode.data());

    vk::Result result = device.createShaderModule(&createInfo, nullptr, &m_vertexShaderModule);
    if (result == vk::Result::eSuccess) {
        m_hasVertexShader = true;
        return true;
    }
    return false;
}

bool VulkanPipeline::createFragmentShaderModule(VulkanContext& context, const std::vector<uint32_t>& spirvCode) {
    if (spirvCode.empty() || m_hasFragmentShader) return false;

    vk::Device device = context.getDevice();
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.setCodeSize(spirvCode.size() * sizeof(uint32_t));
    createInfo.setPCode(spirvCode.data());

    vk::Result result = device.createShaderModule(&createInfo, nullptr, &m_fragmentShaderModule);
    if (result == vk::Result::eSuccess) {
        m_hasFragmentShader = true;
        return true;
    }
    return false;
}

bool VulkanPipeline::createShaderModule(VulkanContext& context, const std::vector<uint32_t>& spirvCode) {
    return createVertexShaderModule(context, spirvCode);
}

bool VulkanPipeline::createDescriptorSetLayout(VulkanContext& context,
                                               const std::vector<vk::DescriptorSetLayoutBinding>& bindings) {
    if (bindings.empty() || m_hasDescriptorSetLayout) return false;

    vk::Device device = context.getDevice();
    vk::DescriptorSetLayoutCreateInfo createInfo{};
    createInfo.setBindingCount(static_cast<uint32_t>(bindings.size()));
    createInfo.setPBindings(bindings.data());

    vk::Result result = device.createDescriptorSetLayout(&createInfo, nullptr, &m_descriptorSetLayout);
    if (result == vk::Result::eSuccess) {
        m_hasDescriptorSetLayout = true;
        return true;
    }
    return false;
}

bool VulkanPipeline::createPipelineLayout(VulkanContext& context) {
    if (m_hasPipelineLayout) return false;

    vk::Device device = context.getDevice();
    vk::PipelineLayoutCreateInfo createInfo{};

    if (m_hasDescriptorSetLayout) {
        createInfo.setSetLayoutCount(1);
        createInfo.setPSetLayouts(&m_descriptorSetLayout);
    }

    vk::Result result = device.createPipelineLayout(&createInfo, nullptr, &m_pipelineLayout);
    if (result == vk::Result::eSuccess) {
        m_hasPipelineLayout = true;
        return true;
    }
    return false;
}

bool VulkanPipeline::createGraphicsPipeline(VulkanContext& context, const PipelineConfig& config) {
    if (m_hasPipeline) return false;
    if (!m_hasVertexShader || !m_hasFragmentShader || !m_hasPipelineLayout) return false;

    vk::Device device = context.getDevice();

    vk::PipelineShaderStageCreateInfo vertexInfo{};
    vertexInfo.setStage(vk::ShaderStageFlagBits::eVertex);
    vertexInfo.setModule(m_vertexShaderModule);
    vertexInfo.setPName("main");

    vk::PipelineShaderStageCreateInfo fragmentInfo{};
    fragmentInfo.setStage(vk::ShaderStageFlagBits::eFragment);
    fragmentInfo.setModule(m_fragmentShaderModule);
    fragmentInfo.setPName("main");

    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertexInfo, fragmentInfo};

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList);

    vk::Viewport viewport{};
    viewport.setX(0.0f);
    viewport.setY(0.0f);
    viewport.setWidth(static_cast<float>(config.extent.width));
    viewport.setHeight(static_cast<float>(config.extent.height));
    viewport.setMinDepth(0.0f);
    viewport.setMaxDepth(1.0f);

    vk::Rect2D scissor{};
    scissor.setOffset({0, 0});
    scissor.setExtent(config.extent);

    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.setViewportCount(1);
    viewportState.setPViewports(&viewport);
    viewportState.setScissorCount(1);
    viewportState.setPScissors(&scissor);

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.setPolygonMode(vk::PolygonMode::eFill);
    rasterizer.setLineWidth(1.0f);
    rasterizer.setCullMode(vk::CullModeFlagBits::eBack);

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.setColorWriteMask(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    colorBlendAttachment.setBlendEnable(true);

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.setAttachmentCount(1);
    colorBlending.setPAttachments(&colorBlendAttachment);

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.setStageCount(2);
    pipelineInfo.setPStages(shaderStages);
    pipelineInfo.setPVertexInputState(&vertexInputInfo);
    pipelineInfo.setPInputAssemblyState(&inputAssembly);
    pipelineInfo.setPViewportState(&viewportState);
    pipelineInfo.setPRasterizationState(&rasterizer);
    pipelineInfo.setPMultisampleState(&multisampling);
    pipelineInfo.setPColorBlendState(&colorBlending);
    pipelineInfo.setLayout(m_pipelineLayout);
    pipelineInfo.setRenderPass(config.renderPass);
    pipelineInfo.setSubpass(0);

    try {
        auto [result, createdPipeline] = device.createGraphicsPipeline(nullptr, pipelineInfo);
        if (result == vk::Result::eSuccess) {
            m_pipeline = createdPipeline;
            m_hasPipeline = true;
            return true;
        }
    } catch (const vk::SystemError&) {
        // Pipeline creation failed
    }

    return false;
}

void VulkanPipeline::cleanup(VulkanContext& context) {
    vk::Device device = context.getDevice();

    if (m_pipeline) {
        device.destroyPipeline(m_pipeline);
        m_pipeline = vk::Pipeline();
        m_hasPipeline = false;
    }
    if (m_pipelineLayout) {
        device.destroyPipelineLayout(m_pipelineLayout);
        m_pipelineLayout = vk::PipelineLayout();
        m_hasPipelineLayout = false;
    }
    if (m_descriptorSetLayout) {
        device.destroyDescriptorSetLayout(m_descriptorSetLayout);
        m_descriptorSetLayout = vk::DescriptorSetLayout();
        m_hasDescriptorSetLayout = false;
    }
    if (m_vertexShaderModule) {
        device.destroyShaderModule(m_vertexShaderModule);
        m_vertexShaderModule = vk::ShaderModule();
        m_hasVertexShader = false;
    }
    if (m_fragmentShaderModule) {
        device.destroyShaderModule(m_fragmentShaderModule);
        m_fragmentShaderModule = vk::ShaderModule();
        m_hasFragmentShader = false;
    }
}
