#include "vulkan_descriptor.hpp"
#include "vulkan_context.hpp"

VulkanDescriptor::~VulkanDescriptor() {
}

bool VulkanDescriptor::createDescriptorPool(VulkanContext& context,
                                            const std::vector<vk::DescriptorPoolSize>& poolSizes,
                                            uint32_t maxSets) {
    if (poolSizes.empty() || m_hasPool) return false;

    vk::Device device = context.getDevice();
    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()));
    poolInfo.setPPoolSizes(poolSizes.data());
    poolInfo.setMaxSets(maxSets);
    poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

    vk::Result result = device.createDescriptorPool(&poolInfo, nullptr, &m_descriptorPool);
    if (result == vk::Result::eSuccess) {
        m_hasPool = true;
        return true;
    }
    return false;
}

bool VulkanDescriptor::createDescriptorSetLayout(VulkanContext& context,
                                                  const std::vector<vk::DescriptorSetLayoutBinding>& bindings) {
    if (bindings.empty() || m_hasLayout) return false;

    vk::Device device = context.getDevice();
    vk::DescriptorSetLayoutCreateInfo createInfo{};
    createInfo.setBindingCount(static_cast<uint32_t>(bindings.size()));
    createInfo.setPBindings(bindings.data());

    vk::Result result = device.createDescriptorSetLayout(&createInfo, nullptr, &m_descriptorSetLayout);
    if (result == vk::Result::eSuccess) {
        m_hasLayout = true;
        return true;
    }
    return false;
}

bool VulkanDescriptor::allocateDescriptorSet(VulkanContext& context) {
    if (m_hasDescriptorSet) return false;
    if (!m_hasPool || !m_hasLayout) return false;

    vk::Device device = context.getDevice();
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.setDescriptorPool(m_descriptorPool);
    allocInfo.setDescriptorSetCount(1);
    allocInfo.setPSetLayouts(&m_descriptorSetLayout);

    vk::Result result = device.allocateDescriptorSets(&allocInfo, &m_descriptorSet);
    if (result == vk::Result::eSuccess) {
        m_hasDescriptorSet = true;
        return true;
    }
    return false;
}

bool VulkanDescriptor::updateUniformBuffer(VulkanContext& context, uint32_t binding,
                                           vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range) {
    if (!m_hasDescriptorSet) return false;

    vk::Device device = context.getDevice();
    vk::DescriptorBufferInfo bufferInfo{};
    bufferInfo.setBuffer(buffer);
    bufferInfo.setOffset(offset);
    bufferInfo.setRange(range);

    vk::WriteDescriptorSet descriptorWrite{};
    descriptorWrite.setDstSet(m_descriptorSet);
    descriptorWrite.setDstBinding(binding);
    descriptorWrite.setDstArrayElement(0);
    descriptorWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);
    descriptorWrite.setDescriptorCount(1);
    descriptorWrite.setPBufferInfo(&bufferInfo);

    device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
    return true;
}

void VulkanDescriptor::cleanup(VulkanContext& context) {
    vk::Device device = context.getDevice();

    if (m_hasDescriptorSet && m_hasPool) {
        device.freeDescriptorSets(m_descriptorPool, 1, &m_descriptorSet);
        m_hasDescriptorSet = false;
    }
    m_descriptorSet = vk::DescriptorSet{};

    if (m_descriptorSetLayout) {
        device.destroyDescriptorSetLayout(m_descriptorSetLayout);
        m_descriptorSetLayout = vk::DescriptorSetLayout{};
        m_hasLayout = false;
    }

    if (m_descriptorPool) {
        device.destroyDescriptorPool(m_descriptorPool);
        m_descriptorPool = vk::DescriptorPool{};
        m_hasPool = false;
    }
}
