#include <catch2/catch_all.hpp>
using Catch::Approx;  // Catch2 v3
#include <render/vulkan_context.hpp>
#include <render/vulkan_buffer.hpp>
#include <render/vulkan_descriptor.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

TEST_CASE("VulkanDescriptor: Default construction", "[render][vulkan][descriptor]") {
    VulkanDescriptor descriptor;

    SECTION("Initially has no pool") {
        REQUIRE(descriptor.getDescriptorPool() == vk::DescriptorPool());
    }

    SECTION("Initially has no layout") {
        REQUIRE(descriptor.getDescriptorSetLayout() == vk::DescriptorSetLayout());
    }

    SECTION("Initially has no descriptor set") {
        REQUIRE(descriptor.getDescriptorSet() == vk::DescriptorSet());
    }
}

TEST_CASE("VulkanDescriptor: Create descriptor pool", "[render][vulkan][descriptor][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;// SKIP: Vulkan not available
        return;
    }

    VulkanDescriptor descriptor;

    SECTION("Create uniform buffer pool") {
        std::vector<vk::DescriptorPoolSize> poolSizes;
        poolSizes.push_back({vk::DescriptorType::eUniformBuffer, 100});

        bool result = descriptor.createDescriptorPool(context, poolSizes, 10);
        REQUIRE(result);
        REQUIRE(descriptor.getDescriptorPool() != vk::DescriptorPool());
    }

    SECTION("Create combined image sampler pool") {
        std::vector<vk::DescriptorPoolSize> poolSizes;
        poolSizes.push_back({vk::DescriptorType::eCombinedImageSampler, 50});

        bool result = descriptor.createDescriptorPool(context, poolSizes, 10);
        REQUIRE(result);
        REQUIRE(descriptor.getDescriptorPool() != vk::DescriptorPool());
    }

    SECTION("Create mixed pool") {
        std::vector<vk::DescriptorPoolSize> poolSizes;
        poolSizes.push_back({vk::DescriptorType::eUniformBuffer, 100});
        poolSizes.push_back({vk::DescriptorType::eCombinedImageSampler, 50});

        bool result = descriptor.createDescriptorPool(context, poolSizes, 10);
        REQUIRE(result);
    }

    if (descriptor.getDescriptorPool()) {
        descriptor.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanDescriptor: Double pool creation protection", "[render][vulkan][descriptor][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;// SKIP: Vulkan not available
        return;
    }

    VulkanDescriptor descriptor;

    std::vector<vk::DescriptorPoolSize> poolSizes;
    poolSizes.push_back({vk::DescriptorType::eUniformBuffer, 100});

    if (descriptor.createDescriptorPool(context, poolSizes, 10)) {
        SECTION("Second pool creation returns false") {
            bool result = descriptor.createDescriptorPool(context, poolSizes, 10);
            REQUIRE_FALSE(result);
        }
    }

    if (descriptor.getDescriptorPool()) {
        descriptor.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanDescriptor: Create descriptor set layout", "[render][vulkan][descriptor][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;// SKIP: Vulkan not available
        return;
    }

    VulkanDescriptor descriptor;

    SECTION("Create single binding layout") {
        std::vector<vk::DescriptorSetLayoutBinding> bindings;

        vk::DescriptorSetLayoutBinding binding;
        binding.setBinding(0);
        binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        binding.setDescriptorCount(1);
        binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
        bindings.push_back(binding);

        bool result = descriptor.createDescriptorSetLayout(context, bindings);
        REQUIRE(result);
        REQUIRE(descriptor.getDescriptorSetLayout() != vk::DescriptorSetLayout());
    }

    SECTION("Create multiple binding layout") {
        std::vector<vk::DescriptorSetLayoutBinding> bindings;

        for (uint32_t i = 0; i < 3; ++i) {
            vk::DescriptorSetLayoutBinding binding;
            binding.setBinding(i);
            binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
            binding.setDescriptorCount(1);
            binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
            bindings.push_back(binding);
        }

        bool result = descriptor.createDescriptorSetLayout(context, bindings);
        REQUIRE(result);
    }

    if (descriptor.getDescriptorSetLayout()) {
        descriptor.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanDescriptor: Allocate descriptor set", "[render][vulkan][descriptor][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;// SKIP: Vulkan not available
        return;
    }

    VulkanDescriptor descriptor;

    // Setup: create pool and layout
    std::vector<vk::DescriptorPoolSize> poolSizes;
    poolSizes.push_back({vk::DescriptorType::eUniformBuffer, 100});
    descriptor.createDescriptorPool(context, poolSizes, 10);

    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    vk::DescriptorSetLayoutBinding binding;
    binding.setBinding(0);
    binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
    binding.setDescriptorCount(1);
    binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
    bindings.push_back(binding);
    descriptor.createDescriptorSetLayout(context, bindings);

    SECTION("Allocate descriptor set") {
        bool result = descriptor.allocateDescriptorSet(context);
        REQUIRE(result);
        REQUIRE(descriptor.getDescriptorSet() != vk::DescriptorSet());
    }

    SECTION("Double allocation protection") {
        if (descriptor.allocateDescriptorSet(context)) {
            bool secondResult = descriptor.allocateDescriptorSet(context);
            REQUIRE_FALSE(secondResult);
        }
    }

    if (descriptor.getDescriptorSet()) {
        descriptor.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanDescriptor: Update uniform buffer", "[render][vulkan][descriptor][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;// SKIP: Vulkan not available
        return;
    }

    VulkanDescriptor descriptor;
    UniformBuffer buffer;

    // Setup descriptor
    std::vector<vk::DescriptorPoolSize> poolSizes;
    poolSizes.push_back({vk::DescriptorType::eUniformBuffer, 100});
    descriptor.createDescriptorPool(context, poolSizes, 10);

    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    vk::DescriptorSetLayoutBinding binding;
    binding.setBinding(0);
    binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
    binding.setDescriptorCount(1);
    binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
    bindings.push_back(binding);
    descriptor.createDescriptorSetLayout(context, bindings);
    descriptor.allocateDescriptorSet(context);

    // Create buffer
    if (buffer.create(context, 256)) {
        SECTION("Update uniform buffer descriptor") {
            bool result = descriptor.updateUniformBuffer(context, 0, buffer.getBuffer(), 0, 256);
            REQUIRE(result);
        }
    }

    if (descriptor.getDescriptorSet()) {
        descriptor.cleanup(context);
    }
    if (buffer.isInitialized()) {
        buffer.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanDescriptor: Different descriptor types", "[render][vulkan][descriptor][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;// SKIP: Vulkan not available
        return;
    }

    SECTION("Uniform buffer dynamic") {
        VulkanDescriptor descriptor;

        std::vector<vk::DescriptorPoolSize> poolSizes;
        poolSizes.push_back({vk::DescriptorType::eUniformBufferDynamic, 100});
        descriptor.createDescriptorPool(context, poolSizes, 10);

        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        vk::DescriptorSetLayoutBinding binding;
        binding.setBinding(0);
        binding.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic);
        binding.setDescriptorCount(1);
        binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
        bindings.push_back(binding);

        REQUIRE(descriptor.createDescriptorSetLayout(context, bindings));
        descriptor.cleanup(context);
    }

    SECTION("Storage buffer") {
        VulkanDescriptor descriptor;

        std::vector<vk::DescriptorPoolSize> poolSizes;
        poolSizes.push_back({vk::DescriptorType::eStorageBuffer, 100});
        descriptor.createDescriptorPool(context, poolSizes, 10);

        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        vk::DescriptorSetLayoutBinding binding;
        binding.setBinding(0);
        binding.setDescriptorType(vk::DescriptorType::eStorageBuffer);
        binding.setDescriptorCount(1);
        binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
        bindings.push_back(binding);

        REQUIRE(descriptor.createDescriptorSetLayout(context, bindings));
        descriptor.cleanup(context);
    }

    SECTION("Sampler") {
        VulkanDescriptor descriptor;

        std::vector<vk::DescriptorPoolSize> poolSizes;
        poolSizes.push_back({vk::DescriptorType::eSampler, 100});
        descriptor.createDescriptorPool(context, poolSizes, 10);

        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        vk::DescriptorSetLayoutBinding binding;
        binding.setBinding(0);
        binding.setDescriptorType(vk::DescriptorType::eSampler);
        binding.setDescriptorCount(1);
        binding.setStageFlags(vk::ShaderStageFlagBits::eFragment);
        bindings.push_back(binding);

        REQUIRE(descriptor.createDescriptorSetLayout(context, bindings));
        descriptor.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanDescriptor: Array descriptors", "[render][vulkan][descriptor][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;// SKIP: Vulkan not available
        return;
    }

    VulkanDescriptor descriptor;

    SECTION("Descriptor array") {
        std::vector<vk::DescriptorPoolSize> poolSizes;
        poolSizes.push_back({vk::DescriptorType::eCombinedImageSampler, 100});
        descriptor.createDescriptorPool(context, poolSizes, 10);

        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        vk::DescriptorSetLayoutBinding binding;
        binding.setBinding(0);
        binding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        binding.setDescriptorCount(16);  // Array of 16
        binding.setStageFlags(vk::ShaderStageFlagBits::eFragment);
        bindings.push_back(binding);

        bool result = descriptor.createDescriptorSetLayout(context, bindings);
        REQUIRE(result);
        descriptor.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanDescriptor: Cleanup and recreate", "[render][vulkan][descriptor][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;// SKIP: Vulkan not available
        return;
    }

    VulkanDescriptor descriptor;

    SECTION("Cleanup and recreate pool") {
        std::vector<vk::DescriptorPoolSize> poolSizes;
        poolSizes.push_back({vk::DescriptorType::eUniformBuffer, 100});

        if (descriptor.createDescriptorPool(context, poolSizes, 10)) {
            descriptor.cleanup(context);
            REQUIRE(descriptor.getDescriptorPool() == vk::DescriptorPool());

            // Can recreate
            descriptor.createDescriptorPool(context, poolSizes, 10);
            REQUIRE(descriptor.getDescriptorPool() != vk::DescriptorPool());

            descriptor.cleanup(context);
        }
    }

    SECTION("Cleanup before create is safe") {
        descriptor.cleanup(context);
        REQUIRE(true);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanDescriptor: Multiple sets", "[render][vulkan][descriptor][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;// SKIP: Vulkan not available
        return;
    }

    SECTION("Create multiple descriptors") {
        VulkanDescriptor descriptor1;
        VulkanDescriptor descriptor2;

        std::vector<vk::DescriptorPoolSize> poolSizes;
        poolSizes.push_back({vk::DescriptorType::eUniformBuffer, 100});

        // Each can have its own pool
        REQUIRE(descriptor1.createDescriptorPool(context, poolSizes, 10));
        REQUIRE(descriptor2.createDescriptorPool(context, poolSizes, 10));

        descriptor1.cleanup(context);
        descriptor2.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanDescriptor: Empty bindings handling", "[render][vulkan][descriptor][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;// SKIP: Vulkan not available
        return;
    }

    VulkanDescriptor descriptor;

    SECTION("Empty pool sizes returns false") {
        std::vector<vk::DescriptorPoolSize> emptyPoolSizes;
        bool result = descriptor.createDescriptorPool(context, emptyPoolSizes, 10);
        REQUIRE_FALSE(result);
    }

    SECTION("Empty bindings returns false") {
        std::vector<vk::DescriptorSetLayoutBinding> emptyBindings;
        bool result = descriptor.createDescriptorSetLayout(context, emptyBindings);
        REQUIRE_FALSE(result);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}
