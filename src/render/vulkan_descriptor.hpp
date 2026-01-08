#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

class VulkanContext;

class VulkanDescriptor {
public:
    VulkanDescriptor() = default;
    ~VulkanDescriptor();

    // Non-copyable
    VulkanDescriptor(const VulkanDescriptor&) = delete;
    VulkanDescriptor& operator=(const VulkanDescriptor&) = delete;

    bool createDescriptorPool(VulkanContext& context,
                              const std::vector<vk::DescriptorPoolSize>& poolSizes,
                              uint32_t maxSets);
    bool createDescriptorSetLayout(VulkanContext& context,
                                    const std::vector<vk::DescriptorSetLayoutBinding>& bindings);
    bool allocateDescriptorSet(VulkanContext& context);
    bool updateUniformBuffer(VulkanContext& context, uint32_t binding,
                             vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range);

    vk::DescriptorPool getDescriptorPool() const { return m_descriptorPool; }
    vk::DescriptorSetLayout getDescriptorSetLayout() const { return m_descriptorSetLayout; }
    vk::DescriptorSet getDescriptorSet() const { return m_descriptorSet; }

    void cleanup(VulkanContext& context);

private:
    vk::DescriptorPool m_descriptorPool;
    vk::DescriptorSetLayout m_descriptorSetLayout;
    vk::DescriptorSet m_descriptorSet;

    bool m_hasPool = false;
    bool m_hasLayout = false;
    bool m_hasDescriptorSet = false;
};
