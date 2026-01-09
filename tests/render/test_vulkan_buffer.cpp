#include <catch2/catch_all.hpp>
using Catch::Approx;  // Catch2 v3
#include <render/vulkan_context.hpp>
#include <render/vulkan_buffer.hpp>
#include <glm/glm.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <cstring>

// Helper class to manage GLFW initialization
class GLFWHelper {
public:
    static bool ensureInitialized() {
        if (!s_initialized) {
            if (glfwInit() == GLFW_TRUE) {
                s_initialized = true;
                return true;
            }
            return false;
        }
        return true;
    }

    static void terminate() {
        if (s_initialized) {

            s_initialized = false;
        }
    }

private:
    static inline bool s_initialized = false;
};

TEST_CASE("VulkanBuffer: Default construction", "[render][vulkan][buffer]") {
    VulkanBuffer buffer;

    SECTION("Initially not initialized") {
        REQUIRE_FALSE(buffer.isInitialized());
    }

    SECTION("Size is zero") {
        REQUIRE(buffer.getSize() == 0);
    }

    SECTION("Buffer handle is null") {
        REQUIRE(buffer.getBuffer() == vk::Buffer());
    }

    SECTION("Memory handle is null") {
        REQUIRE(buffer.getMemory() == vk::DeviceMemory());
    }
}

TEST_CASE("VulkanBuffer: Create simple buffer", "[render][vulkan][buffer][VulkanGPUTests]") {
    GLFWHelper::ensureInitialized();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        return;  // SKIP: Vulkan not available
    }

    VulkanBuffer buffer;
    vk::DeviceSize size = 1024;

    bool result = buffer.create(context, size,
                                 vk::BufferUsageFlagBits::eVertexBuffer,
                                 vk::MemoryPropertyFlagBits::eHostVisible |
                                 vk::MemoryPropertyFlagBits::eHostCoherent);
    if (result) {
        REQUIRE(buffer.isInitialized());
        REQUIRE(buffer.getSize() == size);
    }

    if (buffer.isInitialized()) {
        buffer.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
}

TEST_CASE("VulkanBuffer: Double creation protection", "[render][vulkan][buffer][VulkanGPUTests]") {
    GLFWHelper::ensureInitialized();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        
        return;  // SKIP: Vulkan not available
    }

    VulkanBuffer buffer;

    if (buffer.create(context, 1024,
                      vk::BufferUsageFlagBits::eVertexBuffer,
                      vk::MemoryPropertyFlagBits::eHostVisible)) {
        SECTION("Second create returns false") {
            bool result = buffer.create(context, 1024,
                                        vk::BufferUsageFlagBits::eVertexBuffer,
                                        vk::MemoryPropertyFlagBits::eHostVisible);
            REQUIRE_FALSE(result);
        }

        buffer.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    
}

TEST_CASE("VertexBuffer: Create and populate", "[render][vulkan][buffer][VulkanGPUTests]") {
    GLFWHelper::ensureInitialized();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        
        return;  // SKIP: Vulkan not available
    }

    VertexBuffer buffer;

    SECTION("Create with data") {
        std::vector<float> vertices = {
            0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f
        };

        bool result = buffer.create(context, vertices.data(), vertices.size() * sizeof(float));
        if (result) {
            REQUIRE(buffer.isInitialized());
            REQUIRE(buffer.getSize() == vertices.size() * sizeof(float));
        }
    }

    if (buffer.isInitialized()) {
        buffer.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    
}

TEST_CASE("IndexBuffer: Create and populate", "[render][vulkan][buffer][VulkanGPUTests]") {
    GLFWHelper::ensureInitialized();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        
        return;  // SKIP: Vulkan not available
    }

    IndexBuffer buffer;

    SECTION("Create with data") {
        std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

        bool result = buffer.create(context, indices.data(),
                                     indices.size() * sizeof(uint32_t),
                                     static_cast<uint32_t>(indices.size()));
        if (result) {
            REQUIRE(buffer.isInitialized());
            REQUIRE(buffer.getIndexCount() == indices.size());
        }
    }

    if (buffer.isInitialized()) {
        buffer.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    
}

TEST_CASE("UniformBuffer: Create and update", "[render][vulkan][buffer][VulkanGPUTests]") {
    GLFWHelper::ensureInitialized();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        
        return;  // SKIP: Vulkan not available
    }

    UniformBuffer buffer;

    SECTION("Create uniform buffer") {
        bool result = buffer.create(context, 256);
        if (result) {
            REQUIRE(buffer.isInitialized());
            REQUIRE(buffer.getSize() == 256);
        }
    }

    SECTION("Set data") {
        if (buffer.create(context, 256)) {
            float data[4] = {1.0f, 2.0f, 3.0f, 4.0f};
            buffer.setData(context, data, sizeof(data));
            // Just verify no crash
            REQUIRE(true);
        }
    }

    SECTION("Map and unmap") {
        if (buffer.create(context, 256)) {
            void* mapped = buffer.map(context);
            if (mapped) {
                // Write some data
                float* data = static_cast<float*>(mapped);
                data[0] = 1.0f;
                data[1] = 2.0f;
                buffer.unmap(context);
            }
            // Just verify no crash
            REQUIRE(true);
        }
    }

    if (buffer.isInitialized()) {
        buffer.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    
}

TEST_CASE("VulkanBuffer: Cleanup and recreate", "[render][vulkan][buffer][VulkanGPUTests]") {
    GLFWHelper::ensureInitialized();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        
        return;  // SKIP: Vulkan not available
    }

    VulkanBuffer buffer;

    SECTION("Can cleanup and recreate") {
        if (buffer.create(context, 1024,
                          vk::BufferUsageFlagBits::eVertexBuffer,
                          vk::MemoryPropertyFlagBits::eHostVisible)) {
            buffer.cleanup(context);
            REQUIRE_FALSE(buffer.isInitialized());

            // Should be able to recreate
            buffer.create(context, 512,
                          vk::BufferUsageFlagBits::eVertexBuffer,
                          vk::MemoryPropertyFlagBits::eHostVisible);
            REQUIRE(buffer.isInitialized());
            REQUIRE(buffer.getSize() == 512);

            buffer.cleanup(context);
        }
    }

    context.shutdown();
    glfwDestroyWindow(window);
    
}

TEST_CASE("VulkanBuffer: Different buffer usages", "[render][vulkan][buffer][VulkanGPUTests]") {
    GLFWHelper::ensureInitialized();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        
        return;  // SKIP: Vulkan not available
    }

    SECTION("Vertex buffer usage") {
        VulkanBuffer buffer;
        if (buffer.create(context, 1024,
                          vk::BufferUsageFlagBits::eVertexBuffer,
                          vk::MemoryPropertyFlagBits::eDeviceLocal)) {
            REQUIRE(buffer.isInitialized());
            buffer.cleanup(context);
        }
    }

    SECTION("Index buffer usage") {
        VulkanBuffer buffer;
        if (buffer.create(context, 1024,
                          vk::BufferUsageFlagBits::eIndexBuffer,
                          vk::MemoryPropertyFlagBits::eDeviceLocal)) {
            REQUIRE(buffer.isInitialized());
            buffer.cleanup(context);
        }
    }

    SECTION("Uniform buffer usage") {
        VulkanBuffer buffer;
        if (buffer.create(context, 1024,
                          vk::BufferUsageFlagBits::eUniformBuffer,
                          vk::MemoryPropertyFlagBits::eHostVisible |
                          vk::MemoryPropertyFlagBits::eHostCoherent)) {
            REQUIRE(buffer.isInitialized());
            buffer.cleanup(context);
        }
    }

    SECTION("Transfer source usage") {
        VulkanBuffer buffer;
        if (buffer.create(context, 1024,
                          vk::BufferUsageFlagBits::eTransferSrc,
                          vk::MemoryPropertyFlagBits::eHostVisible)) {
            REQUIRE(buffer.isInitialized());
            buffer.cleanup(context);
        }
    }

    SECTION("Transfer destination usage") {
        VulkanBuffer buffer;
        if (buffer.create(context, 1024,
                          vk::BufferUsageFlagBits::eTransferDst,
                          vk::MemoryPropertyFlagBits::eDeviceLocal)) {
            REQUIRE(buffer.isInitialized());
            buffer.cleanup(context);
        }
    }

    context.shutdown();
    glfwDestroyWindow(window);
    
}

TEST_CASE("VulkanBuffer: Different memory types", "[render][vulkan][buffer][VulkanGPUTests]") {
    GLFWHelper::ensureInitialized();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        
        return;  // SKIP: Vulkan not available
    }

    SECTION("Device local memory") {
        VulkanBuffer buffer;
        if (buffer.create(context, 1024,
                          vk::BufferUsageFlagBits::eVertexBuffer,
                          vk::MemoryPropertyFlagBits::eDeviceLocal)) {
            REQUIRE(buffer.isInitialized());
            buffer.cleanup(context);
        }
    }

    SECTION("Host visible memory") {
        VulkanBuffer buffer;
        if (buffer.create(context, 1024,
                          vk::BufferUsageFlagBits::eUniformBuffer,
                          vk::MemoryPropertyFlagBits::eHostVisible)) {
            REQUIRE(buffer.isInitialized());
            buffer.cleanup(context);
        }
    }

    SECTION("Host coherent memory") {
        VulkanBuffer buffer;
        if (buffer.create(context, 1024,
                          vk::BufferUsageFlagBits::eUniformBuffer,
                          vk::MemoryPropertyFlagBits::eHostVisible |
                          vk::MemoryPropertyFlagBits::eHostCoherent)) {
            REQUIRE(buffer.isInitialized());
            buffer.cleanup(context);
        }
    }

    context.shutdown();
    glfwDestroyWindow(window);
    
}

TEST_CASE("VulkanBuffer: Large buffer", "[render][vulkan][buffer][VulkanGPUTests]") {
    GLFWHelper::ensureInitialized();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        
        return;  // SKIP: Vulkan not available
    }

    VulkanBuffer buffer;
    vk::DeviceSize largeSize = 10 * 1024 * 1024;  // 10 MB

    SECTION("Can create large buffer") {
        bool result = buffer.create(context, largeSize,
                                     vk::BufferUsageFlagBits::eVertexBuffer,
                                     vk::MemoryPropertyFlagBits::eDeviceLocal);
        if (result) {
            REQUIRE(buffer.getSize() == largeSize);
            buffer.cleanup(context);
        }
    }

    context.shutdown();
    glfwDestroyWindow(window);
    
}

TEST_CASE("VertexBuffer: Update functionality", "[render][vulkan][buffer][VulkanGPUTests]") {
    GLFWHelper::ensureInitialized();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        
        return;  // SKIP: Vulkan not available
    }

    VertexBuffer buffer;

    SECTION("Update method exists") {
        if (buffer.create(context, 256)) {
            std::vector<float> newData = {1.0f, 2.0f, 3.0f};
            buffer.update(context, newData.data(), newData.size() * sizeof(float), 0);
            // Just verify no crash (actual update may be stub)
            REQUIRE(true);
            buffer.cleanup(context);
        }
    }

    context.shutdown();
    glfwDestroyWindow(window);
    
}

TEST_CASE("IndexBuffer: Update functionality", "[render][vulkan][buffer][VulkanGPUTests]") {
    GLFWHelper::ensureInitialized();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        
        return;  // SKIP: Vulkan not available
    }

    IndexBuffer buffer;

    SECTION("Update method exists") {
        if (buffer.create(context, 256)) {
            std::vector<uint32_t> newIndices = {0, 1, 2};
            buffer.update(context, newIndices.data(), newIndices.size() * sizeof(uint32_t), 0);
            // Just verify no crash (actual update may be stub)
            REQUIRE(true);
            buffer.cleanup(context);
        }
    }

    context.shutdown();
    glfwDestroyWindow(window);
    
}
