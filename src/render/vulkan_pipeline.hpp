#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

class VulkanContext;

struct PipelineConfig {
    vk::RenderPass renderPass;
    vk::Extent2D extent;
};

class VulkanPipeline {
public:
    VulkanPipeline() = default;
    ~VulkanPipeline();

    // Non-copyable
    VulkanPipeline(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;

    bool createShaderModule(VulkanContext& context, const std::vector<uint32_t>& spirvCode);
    bool createVertexShaderModule(VulkanContext& context, const std::vector<uint32_t>& spirvCode);
    bool createFragmentShaderModule(VulkanContext& context, const std::vector<uint32_t>& spirvCode);
    bool createDescriptorSetLayout(VulkanContext& context,
                                    const std::vector<vk::DescriptorSetLayoutBinding>& bindings);
    bool createPipelineLayout(VulkanContext& context);
    bool createGraphicsPipeline(VulkanContext& context, const PipelineConfig& config);

    vk::Pipeline getPipeline() const { return m_pipeline; }
    vk::PipelineLayout getPipelineLayout() const { return m_pipelineLayout; }
    vk::DescriptorSetLayout getDescriptorSetLayout() const { return m_descriptorSetLayout; }
    vk::ShaderModule getVertexShaderModule() const { return m_vertexShaderModule; }
    vk::ShaderModule getFragmentShaderModule() const { return m_fragmentShaderModule; }

    void cleanup(VulkanContext& context);

private:
    vk::ShaderModule m_vertexShaderModule;
    vk::ShaderModule m_fragmentShaderModule;
    vk::DescriptorSetLayout m_descriptorSetLayout;
    vk::PipelineLayout m_pipelineLayout;
    vk::Pipeline m_pipeline;

    bool m_hasVertexShader = false;
    bool m_hasFragmentShader = false;
    bool m_hasDescriptorSetLayout = false;
    bool m_hasPipelineLayout = false;
    bool m_hasPipeline = false;
};
