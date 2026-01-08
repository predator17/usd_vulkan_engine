#include "vulkan_pipeline.hpp"
#include "vulkan_context.hpp"
#include "shader.hpp"
#include <spdlog/spdlog.h>

VulkanPipeline::~VulkanPipeline() {
}

VulkanPipeline::VulkanPipeline(VulkanPipeline&& other) noexcept
    : m_vertexShader(other.m_vertexShader)
    , m_fragmentShader(other.m_fragmentShader)
    , m_vertexShaderModule(other.m_vertexShaderModule)
    , m_fragmentShaderModule(other.m_fragmentShaderModule)
    , m_descriptorSetLayout(other.m_descriptorSetLayout)
    , m_pipelineLayout(other.m_pipelineLayout)
    , m_pipeline(other.m_pipeline)
    , m_hasVertexShader(other.m_hasVertexShader)
    , m_hasFragmentShader(other.m_hasFragmentShader)
    , m_hasDescriptorSetLayout(other.m_hasDescriptorSetLayout)
    , m_hasPipelineLayout(other.m_hasPipelineLayout)
    , m_hasPipeline(other.m_hasPipeline)
    , m_usesShaderObjects(other.m_usesShaderObjects)
{
    other.m_vertexShader = nullptr;
    other.m_fragmentShader = nullptr;
    other.m_vertexShaderModule = vk::ShaderModule();
    other.m_fragmentShaderModule = vk::ShaderModule();
    other.m_descriptorSetLayout = vk::DescriptorSetLayout();
    other.m_pipelineLayout = vk::PipelineLayout();
    other.m_pipeline = vk::Pipeline();
    other.m_hasVertexShader = false;
    other.m_hasFragmentShader = false;
    other.m_hasDescriptorSetLayout = false;
    other.m_hasPipelineLayout = false;
    other.m_hasPipeline = false;
    other.m_usesShaderObjects = false;
}

VulkanPipeline& VulkanPipeline::operator=(VulkanPipeline&& other) noexcept {
    if (this != &other) {
        m_vertexShader = other.m_vertexShader;
        m_fragmentShader = other.m_fragmentShader;
        m_vertexShaderModule = other.m_vertexShaderModule;
        m_fragmentShaderModule = other.m_fragmentShaderModule;
        m_descriptorSetLayout = other.m_descriptorSetLayout;
        m_pipelineLayout = other.m_pipelineLayout;
        m_pipeline = other.m_pipeline;
        m_hasVertexShader = other.m_hasVertexShader;
        m_hasFragmentShader = other.m_hasFragmentShader;
        m_hasDescriptorSetLayout = other.m_hasDescriptorSetLayout;
        m_hasPipelineLayout = other.m_hasPipelineLayout;
        m_hasPipeline = other.m_hasPipeline;
        m_usesShaderObjects = other.m_usesShaderObjects;

        other.m_vertexShader = nullptr;
        other.m_fragmentShader = nullptr;
        other.m_vertexShaderModule = vk::ShaderModule();
        other.m_fragmentShaderModule = vk::ShaderModule();
        other.m_descriptorSetLayout = vk::DescriptorSetLayout();
        other.m_pipelineLayout = vk::PipelineLayout();
        other.m_pipeline = vk::Pipeline();
        other.m_hasVertexShader = false;
        other.m_hasFragmentShader = false;
        other.m_hasDescriptorSetLayout = false;
        other.m_hasPipelineLayout = false;
        other.m_hasPipeline = false;
        other.m_usesShaderObjects = false;
    }
    return *this;
}

bool VulkanPipeline::attachVertexShader(VulkanContext& context, Shader& shader) {
    if (!shader.isInitialized() || shader.getStage() != Shader::Stage::Vertex) {
        SPDLOG_ERROR("VulkanPipeline: Invalid vertex shader");
        return false;
    }
    m_vertexShader = &shader;
    m_hasVertexShader = true;
    m_usesShaderObjects = true;
    return true;
}

bool VulkanPipeline::attachFragmentShader(VulkanContext& context, Shader& shader) {
    if (!shader.isInitialized() || shader.getStage() != Shader::Stage::Fragment) {
        SPDLOG_ERROR("VulkanPipeline: Invalid fragment shader");
        return false;
    }
    m_fragmentShader = &shader;
    m_hasFragmentShader = true;
    m_usesShaderObjects = true;
    return true;
}

bool VulkanPipeline::attachShader(VulkanContext& context, Shader& shader) {
    if (!shader.isInitialized()) {
        SPDLOG_ERROR("VulkanPipeline: Shader not initialized");
        return false;
    }

    switch (shader.getStage()) {
        case Shader::Stage::Vertex:
            return attachVertexShader(context, shader);
        case Shader::Stage::Fragment:
            return attachFragmentShader(context, shader);
        default:
            SPDLOG_ERROR("VulkanPipeline: Unsupported shader stage for pipeline");
            return false;
    }
}

vk::ShaderModule VulkanPipeline::getVertexShaderModule() const {
    if (m_usesShaderObjects && m_vertexShader) {
        return m_vertexShader->getModule();
    }
    return m_vertexShaderModule;
}

vk::ShaderModule VulkanPipeline::getFragmentShaderModule() const {
    if (m_usesShaderObjects && m_fragmentShader) {
        return m_fragmentShader->getModule();
    }
    return m_fragmentShaderModule;
}

bool VulkanPipeline::checkForShaderReload(VulkanContext& context, const PipelineConfig& config) {
    if (!m_usesShaderObjects || !m_hasPipeline) {
        return false;
    }

    bool reloaded = false;
    if (m_vertexShader && m_vertexShader->checkForReload(context)) {
        reloaded = true;
    }
    if (m_fragmentShader && m_fragmentShader->checkForReload(context)) {
        reloaded = true;
    }

    // If shaders were reloaded, recreate the pipeline
    if (reloaded) {
        SPDLOG_INFO("VulkanPipeline: Shaders reloaded, recreating pipeline");
        vk::Device device = context.getDevice();
        if (m_pipeline) {
            device.destroyPipeline(m_pipeline);
            m_pipeline = vk::Pipeline();
            m_hasPipeline = false;
        }
        return createGraphicsPipeline(context, config);
    }

    return false;
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

bool VulkanPipeline::createPipelineLayout(VulkanContext& context, const std::vector<vk::PushConstantRange>& pushConstantRanges) {
    if (m_hasPipelineLayout) return false;

    vk::Device device = context.getDevice();
    vk::PipelineLayoutCreateInfo createInfo{};

    if (m_hasDescriptorSetLayout) {
        createInfo.setSetLayoutCount(1);
        createInfo.setPSetLayouts(&m_descriptorSetLayout);
    }

    createInfo.setPushConstantRangeCount(static_cast<uint32_t>(pushConstantRanges.size()));
    createInfo.setPPushConstantRanges(pushConstantRanges.data());

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

    // Get shader modules (from Shader objects or direct modules)
    vk::ShaderModule vertModule = getVertexShaderModule();
    vk::ShaderModule fragModule = getFragmentShaderModule();

    if (!vertModule || !fragModule) {
        SPDLOG_ERROR("VulkanPipeline: Invalid shader modules");
        return false;
    }

    vk::PipelineShaderStageCreateInfo vertexInfo{};
    vertexInfo.setStage(vk::ShaderStageFlagBits::eVertex);
    vertexInfo.setModule(vertModule);
    vertexInfo.setPName("main");

    vk::PipelineShaderStageCreateInfo fragmentInfo{};
    fragmentInfo.setStage(vk::ShaderStageFlagBits::eFragment);
    fragmentInfo.setModule(fragModule);
    fragmentInfo.setPName("main");

    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertexInfo, fragmentInfo};

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &config.vertexBinding;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(config.vertexAttributes.size());
    vertexInputInfo.pVertexAttributeDescriptions = config.vertexAttributes.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;

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
    rasterizer.setFrontFace(vk::FrontFace::eCounterClockwise);

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);

    // Depth stencil state - critical for proper rendering
    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.setDepthTestEnable(true);
    depthStencil.setDepthWriteEnable(true);
    depthStencil.setDepthCompareOp(vk::CompareOp::eLessOrEqual);
    depthStencil.setDepthBoundsTestEnable(false);
    depthStencil.setStencilTestEnable(false);

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.setColorWriteMask(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    colorBlendAttachment.setBlendEnable(false);

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
    pipelineInfo.setPDepthStencilState(&depthStencil);
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
    // Only destroy shader modules if they're not owned by Shader objects
    if (!m_usesShaderObjects) {
        if (m_vertexShaderModule) {
            device.destroyShaderModule(m_vertexShaderModule);
            m_vertexShaderModule = vk::ShaderModule();
        }
        if (m_fragmentShaderModule) {
            device.destroyShaderModule(m_fragmentShaderModule);
            m_fragmentShaderModule = vk::ShaderModule();
        }
    }
    // Reset shader references but don't destroy the Shader objects
    m_vertexShader = nullptr;
    m_fragmentShader = nullptr;
    m_hasVertexShader = false;
    m_hasFragmentShader = false;
    m_usesShaderObjects = false;
}
