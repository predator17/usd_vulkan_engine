#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>

class VulkanContext;
class Shader;

struct PipelineConfig {
    vk::RenderPass renderPass;
    vk::Extent2D extent;
    vk::VertexInputBindingDescription vertexBinding;
    std::vector<vk::VertexInputAttributeDescription> vertexAttributes;
};

class VulkanPipeline {
public:
    VulkanPipeline() = default;
    ~VulkanPipeline();

    // Non-copyable
    VulkanPipeline(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;

    // Movable
    VulkanPipeline(VulkanPipeline&& other) noexcept;
    VulkanPipeline& operator=(VulkanPipeline&& other) noexcept;

    // Shader integration - attach Shader objects
    bool attachVertexShader(VulkanContext& context, Shader& shader);
    bool attachFragmentShader(VulkanContext& context, Shader& shader);
    bool attachShader(VulkanContext& context, Shader& shader);

    // Legacy methods for backward compatibility
    bool createShaderModule(VulkanContext& context, const std::vector<uint32_t>& spirvCode);
    bool createVertexShaderModule(VulkanContext& context, const std::vector<uint32_t>& spirvCode);
    bool createFragmentShaderModule(VulkanContext& context, const std::vector<uint32_t>& spirvCode);

    bool createDescriptorSetLayout(VulkanContext& context,
                                    const std::vector<vk::DescriptorSetLayoutBinding>& bindings);
    bool createPipelineLayout(VulkanContext& context);
    bool createPipelineLayout(VulkanContext& context, const std::vector<vk::PushConstantRange>& pushConstantRanges);
    bool createGraphicsPipeline(VulkanContext& context, const PipelineConfig& config);

    vk::Pipeline getPipeline() const { return m_pipeline; }
    vk::PipelineLayout getPipelineLayout() const { return m_pipelineLayout; }
    vk::DescriptorSetLayout getDescriptorSetLayout() const { return m_descriptorSetLayout; }
    vk::ShaderModule getVertexShaderModule() const;
    vk::ShaderModule getFragmentShaderModule() const;

    // Check for hot-reload and update pipeline if needed
    bool checkForShaderReload(VulkanContext& context, const PipelineConfig& config);

    void cleanup(VulkanContext& context);

private:
    // Shader references (non-owning)
    Shader* m_vertexShader = nullptr;
    Shader* m_fragmentShader = nullptr;

    // Legacy direct shader module management
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
    bool m_usesShaderObjects = false;  // True if using Shader objects
};
