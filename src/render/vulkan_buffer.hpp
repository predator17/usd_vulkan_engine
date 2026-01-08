#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>

class VulkanContext;

class VulkanBuffer {
public:
    VulkanBuffer() = default;
    virtual ~VulkanBuffer();

    // Non-copyable
    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    void cleanup(VulkanContext& context);
    vk::Buffer getBuffer() const { return m_buffer; }
    vk::DeviceMemory getMemory() const { return m_memory; }
    vk::DeviceSize getSize() const { return m_size; }
    bool isInitialized() const { return m_initialized; }

protected:
    bool createBuffer(VulkanContext& context, vk::DeviceSize size,
                      vk::BufferUsageFlags usage,
                      vk::MemoryPropertyFlags properties);

    vk::Buffer m_buffer;
    vk::DeviceMemory m_memory;
    vk::DeviceSize m_size = 0;
    bool m_initialized = false;
};

class VertexBuffer : public VulkanBuffer {
public:
    VertexBuffer() = default;
    bool create(VulkanContext& context, vk::DeviceSize size);
    bool create(VulkanContext& context, const void* data, vk::DeviceSize size);
    void update(VulkanContext& context, const void* data, vk::DeviceSize size, vk::DeviceSize offset = 0);
};

class IndexBuffer : public VulkanBuffer {
public:
    IndexBuffer() = default;
    uint32_t getIndexCount() const { return m_indexCount; }
    bool create(VulkanContext& context, vk::DeviceSize size);
    bool create(VulkanContext& context, const void* data, vk::DeviceSize size, uint32_t indexCount);
    void update(VulkanContext& context, const void* data, vk::DeviceSize size, vk::DeviceSize offset = 0);
private:
    uint32_t m_indexCount = 0;
};

class UniformBuffer : public VulkanBuffer {
public:
    UniformBuffer() = default;
    bool create(VulkanContext& context, vk::DeviceSize size);
    void setData(VulkanContext& context, const void* data, vk::DeviceSize size);
    void* map(VulkanContext& context);
    void unmap(VulkanContext& context);
private:
    void* m_mapped = nullptr;
};
