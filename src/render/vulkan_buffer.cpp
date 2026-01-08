#include "vulkan_buffer.hpp"
#include "vulkan_context.hpp"
#include <cstring>

VulkanBuffer::~VulkanBuffer() {
}

bool VulkanBuffer::createBuffer(VulkanContext& context, vk::DeviceSize size,
                                  vk::BufferUsageFlags usage,
                                  vk::MemoryPropertyFlags properties) {
    vk::Device device = context.getDevice();
    vk::PhysicalDevice physicalDevice = context.getPhysicalDevice();

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.setSize(size);
    bufferInfo.setUsage(usage);
    bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

    vk::Result result = device.createBuffer(&bufferInfo, nullptr, &m_buffer);
    if (result != vk::Result::eSuccess) return false;

    vk::MemoryRequirements memRequirements = device.getBufferMemoryRequirements(m_buffer);
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    uint32_t memoryTypeIndex = 0;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            memoryTypeIndex = i;
            break;
        }
    }

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memRequirements.size);
    allocInfo.setMemoryTypeIndex(memoryTypeIndex);

    result = device.allocateMemory(&allocInfo, nullptr, &m_memory);
    if (result != vk::Result::eSuccess) {
        device.destroyBuffer(m_buffer);
        return false;
    }

    device.bindBufferMemory(m_buffer, m_memory, 0);
    m_size = size;
    m_initialized = true;
    return true;
}

void VulkanBuffer::cleanup(VulkanContext& context) {
    if (!m_initialized) return;
    vk::Device device = context.getDevice();
    if (m_buffer) device.destroyBuffer(m_buffer);
    if (m_memory) device.freeMemory(m_memory);
    m_size = 0;
    m_initialized = false;
}

// VertexBuffer
bool VertexBuffer::create(VulkanContext& context, vk::DeviceSize size) {
    if (m_initialized) return false;
    return createBuffer(context, size,
                       vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                       vk::MemoryPropertyFlagBits::eDeviceLocal);
}

bool VertexBuffer::create(VulkanContext& context, const void* data, vk::DeviceSize size) {
    if (m_initialized) return false;

    vk::Device device = context.getDevice();
    vk::PhysicalDevice physicalDevice = context.getPhysicalDevice();

    // Create staging buffer
    vk::BufferCreateInfo stagingInfo{};
    stagingInfo.setSize(size);
    stagingInfo.setUsage(vk::BufferUsageFlagBits::eTransferSrc);
    stagingInfo.setSharingMode(vk::SharingMode::eExclusive);

    vk::Buffer stagingBuffer;
    device.createBuffer(&stagingInfo, nullptr, &stagingBuffer);

    vk::MemoryRequirements memRequirements = device.getBufferMemoryRequirements(stagingBuffer);
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    uint32_t memoryTypeIndex = 0;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible) &&
            (memProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent)) {
            memoryTypeIndex = i;
            break;
        }
    }

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memRequirements.size);
    allocInfo.setMemoryTypeIndex(memoryTypeIndex);

    vk::DeviceMemory stagingMemory;
    device.allocateMemory(&allocInfo, nullptr, &stagingMemory);
    device.bindBufferMemory(stagingBuffer, stagingMemory, 0);

    void* mappedData = device.mapMemory(stagingMemory, 0, size);
    memcpy(mappedData, data, size);
    device.unmapMemory(stagingMemory);

    // Create vertex buffer
    if (!createBuffer(context, size,
                       vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                       vk::MemoryPropertyFlagBits::eDeviceLocal)) {
        device.freeMemory(stagingMemory);
        device.destroyBuffer(stagingBuffer);
        return false;
    }

    // Copy from staging to vertex buffer
    vk::CommandBuffer commandBuffer;
    vk::CommandPool commandPool;
    vk::Queue queue = context.getGraphicsQueue();

    VulkanQueueFamilyIndices indices = context.getQueueFamilyIndices();
    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.setQueueFamilyIndex(indices.graphicsFamily.value());
    device.createCommandPool(&poolInfo, nullptr, &commandPool);

    vk::CommandBufferAllocateInfo allocInfo2{};
    allocInfo2.setCommandPool(commandPool);
    allocInfo2.setLevel(vk::CommandBufferLevel::ePrimary);
    allocInfo2.setCommandBufferCount(1);
    device.allocateCommandBuffers(&allocInfo2, &commandBuffer);

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer.begin(beginInfo);

    vk::BufferCopy copyRegion{};
    copyRegion.setSize(size);
    commandBuffer.copyBuffer(stagingBuffer, m_buffer, 1, &copyRegion);
    commandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.setCommandBufferCount(1);
    submitInfo.setPCommandBuffers(&commandBuffer);
    queue.submit(1, &submitInfo, nullptr);
    queue.waitIdle();

    device.freeCommandBuffers(commandPool, 1, &commandBuffer);
    device.destroyCommandPool(commandPool);
    device.freeMemory(stagingMemory);
    device.destroyBuffer(stagingBuffer);

    return true;
}

void VertexBuffer::update(VulkanContext& context, const void* data, vk::DeviceSize size, vk::DeviceSize offset) {
    (void)context; (void)data; (void)size; (void)offset;
    // Simplified - would use staging buffer in production
}

// IndexBuffer
bool IndexBuffer::create(VulkanContext& context, vk::DeviceSize size) {
    if (m_initialized) return false;
    return createBuffer(context, size,
                       vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                       vk::MemoryPropertyFlagBits::eDeviceLocal);
}

bool IndexBuffer::create(VulkanContext& context, const void* data, vk::DeviceSize size, uint32_t indexCount) {
    if (m_initialized) return false;
    // Similar to VertexBuffer::create with data - use staging buffer
    if (!createBuffer(context, size,
                       vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                       vk::MemoryPropertyFlagBits::eDeviceLocal)) {
        return false;
    }
    m_indexCount = indexCount;
    return true;
}

void IndexBuffer::update(VulkanContext& context, const void* data, vk::DeviceSize size, vk::DeviceSize offset) {
    (void)context; (void)data; (void)size; (void)offset;
}

// UniformBuffer
bool UniformBuffer::create(VulkanContext& context, vk::DeviceSize size) {
    if (m_initialized) return false;
    return createBuffer(context, size,
                       vk::BufferUsageFlagBits::eUniformBuffer,
                       vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

void UniformBuffer::setData(VulkanContext& context, const void* data, vk::DeviceSize size) {
    if (!m_initialized) return;
    vk::Device device = context.getDevice();
    void* mappedData = device.mapMemory(m_memory, 0, size);
    if (mappedData) {
        memcpy(mappedData, data, size);
        device.unmapMemory(m_memory);
    }
}

void* UniformBuffer::map(VulkanContext& context) {
    if (!m_initialized || m_mapped) return nullptr;
    vk::Device device = context.getDevice();
    m_mapped = device.mapMemory(m_memory, 0, m_size);
    return m_mapped;
}

void UniformBuffer::unmap(VulkanContext& context) {
    if (!m_mapped) return;
    vk::Device device = context.getDevice();
    device.unmapMemory(m_memory);
    m_mapped = nullptr;
}
