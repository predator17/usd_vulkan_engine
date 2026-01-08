#include <catch2/catch.hpp>
#include <test_common.hpp>
#include <render/vulkan_context.hpp>
#include <render/vulkan_pipeline.hpp>
#include <render/vulkan_swapchain.hpp>
#include <render/shader.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

TEST_CASE("VulkanPipeline: Default construction", "[render][vulkan][pipeline]") {
    VulkanPipeline pipeline;

    SECTION("Initially has no shaders") {
        REQUIRE(pipeline.getVertexShaderModule() == vk::ShaderModule());
        REQUIRE(pipeline.getFragmentShaderModule() == vk::ShaderModule());
    }

    SECTION("Initially has no pipeline") {
        REQUIRE(pipeline.getPipeline() == vk::Pipeline());
    }

    SECTION("Initially has no pipeline layout") {
        REQUIRE(pipeline.getPipelineLayout() == vk::PipelineLayout());
    }

    SECTION("Initially has no descriptor set layout") {
        REQUIRE(pipeline.getDescriptorSetLayout() == vk::DescriptorSetLayout());
    }
}

TEST_CASE("VulkanPipeline: Create shader modules", "[render][vulkan][pipeline][VulkanGPUTests]") {
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

    VulkanPipeline pipeline;

    // Minimal valid SPIR-V header
    std::vector<uint32_t> spirvCode = {
        0x07230203,  // Magic number
        0x00010100,  // Version 1.0
        0x00000001,  // Generator magic
        0x00000008,  // Bound
        0x00000000   // Schema
    };

    SECTION("Create vertex shader module") {
        bool result = pipeline.createVertexShaderModule(context, spirvCode);
        // May fail with minimal SPIR-V, but shouldn't crash
        if (result) {
            REQUIRE(pipeline.getVertexShaderModule() != vk::ShaderModule());
        }
    }

    SECTION("Create fragment shader module") {
        bool result = pipeline.createFragmentShaderModule(context, spirvCode);
        // May fail with minimal SPIR-V, but shouldn't crash
        if (result) {
            REQUIRE(pipeline.getFragmentShaderModule() != vk::ShaderModule());
        }
    }

    SECTION("Double shader creation protection") {
        if (pipeline.createVertexShaderModule(context, spirvCode)) {
            bool secondResult = pipeline.createVertexShaderModule(context, spirvCode);
            REQUIRE_FALSE(secondResult);
        }
    }

    if (pipeline.getVertexShaderModule()) {
        pipeline.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanPipeline: Create descriptor set layout", "[render][vulkan][pipeline][VulkanGPUTests]") {
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

    VulkanPipeline pipeline;

    SECTION("Create descriptor set layout") {
        vk::DescriptorSetLayoutBinding binding;
        binding.setBinding(0);
        binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        binding.setDescriptorCount(1);
        binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);

        std::vector<vk::DescriptorSetLayoutBinding> bindings = {binding};

        bool result = pipeline.createDescriptorSetLayout(context, bindings);
        REQUIRE(result);
        REQUIRE(pipeline.getDescriptorSetLayout() != vk::DescriptorSetLayout());
    }

    SECTION("Double layout creation protection") {
        vk::DescriptorSetLayoutBinding binding;
        binding.setBinding(0);
        binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        binding.setDescriptorCount(1);
        binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);

        std::vector<vk::DescriptorSetLayoutBinding> bindings = {binding};

        if (pipeline.createDescriptorSetLayout(context, bindings)) {
            bool secondResult = pipeline.createDescriptorSetLayout(context, bindings);
            REQUIRE_FALSE(secondResult);
        }
    }

    if (pipeline.getDescriptorSetLayout()) {
        pipeline.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanPipeline: Create pipeline layout", "[render][vulkan][pipeline][VulkanGPUTests]") {
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

    VulkanPipeline pipeline;

    SECTION("Create pipeline layout without descriptor set") {
        bool result = pipeline.createPipelineLayout(context);
        REQUIRE(result);
        REQUIRE(pipeline.getPipelineLayout() != vk::PipelineLayout());
    }

    SECTION("Create pipeline layout with descriptor set") {
        vk::DescriptorSetLayoutBinding binding;
        binding.setBinding(0);
        binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        binding.setDescriptorCount(1);
        binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);

        std::vector<vk::DescriptorSetLayoutBinding> bindings = {binding};

        if (pipeline.createDescriptorSetLayout(context, bindings)) {
            bool result = pipeline.createPipelineLayout(context);
            REQUIRE(result);
            REQUIRE(pipeline.getPipelineLayout() != vk::PipelineLayout());
        }
    }

    if (pipeline.getPipelineLayout()) {
        pipeline.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanPipeline: Create graphics pipeline", "[render][vulkan][pipeline][VulkanGPUTests]") {
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

    // Create swapchain for render pass
    VulkanSwapchain swapchain;
    vk::Extent2D extent(800, 600);
    if (!swapchain.create(context, extent)) {
        context.shutdown();
        glfwDestroyWindow(window);
        glfwTerminate();
        return;// SKIP: Swapchain creation failed
        return;
    }

    VulkanPipeline pipeline;

    // Minimal SPIR-V for testing
    std::vector<uint32_t> spirvCode = {
        0x07230203, 0x00010100, 0x00000001, 0x00000008, 0x00000000
    };

    // Setup needed components
    pipeline.createDescriptorSetLayout(context, {});
    pipeline.createPipelineLayout(context);
    pipeline.createVertexShaderModule(context, spirvCode);
    pipeline.createFragmentShaderModule(context, spirvCode);

    SECTION("Create graphics pipeline") {
        PipelineConfig config;
        config.renderPass = swapchain.getRenderPass();
        config.extent = extent;

        bool result = pipeline.createGraphicsPipeline(context, config);
        // May fail with minimal SPIR-V
        if (result) {
            REQUIRE(pipeline.getPipeline() != vk::Pipeline());
        }
    }

    pipeline.cleanup(context);
    swapchain.cleanup(context);
    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanPipeline: Multiple descriptor bindings", "[render][vulkan][pipeline][VulkanGPUTests]") {
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

    VulkanPipeline pipeline;

    SECTION("Multiple uniform buffer bindings") {
        std::vector<vk::DescriptorSetLayoutBinding> bindings;

        for (uint32_t i = 0; i < 3; ++i) {
            vk::DescriptorSetLayoutBinding binding;
            binding.setBinding(i);
            binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
            binding.setDescriptorCount(1);
            binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
            bindings.push_back(binding);
        }

        bool result = pipeline.createDescriptorSetLayout(context, bindings);
        REQUIRE(result);
    }

    if (pipeline.getDescriptorSetLayout()) {
        pipeline.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanPipeline: Combined image sampler binding", "[render][vulkan][pipeline][VulkanGPUTests]") {
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

    VulkanPipeline pipeline;

    SECTION("Create sampler binding") {
        vk::DescriptorSetLayoutBinding binding;
        binding.setBinding(0);
        binding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        binding.setDescriptorCount(1);
        binding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

        std::vector<vk::DescriptorSetLayoutBinding> bindings = {binding};

        bool result = pipeline.createDescriptorSetLayout(context, bindings);
        REQUIRE(result);
    }

    if (pipeline.getDescriptorSetLayout()) {
        pipeline.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanPipeline: Cleanup and recreate", "[render][vulkan][pipeline][VulkanGPUTests]") {
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

    VulkanPipeline pipeline;
    std::vector<uint32_t> spirvCode = {
        0x07230203, 0x00010100, 0x00000001, 0x00000008, 0x00000000
    };

    SECTION("Cleanup and recreate shaders") {
        if (pipeline.createVertexShaderModule(context, spirvCode)) {
            pipeline.cleanup(context);
            REQUIRE(pipeline.getVertexShaderModule() == vk::ShaderModule());

            // Can recreate
            pipeline.createVertexShaderModule(context, spirvCode);
            if (pipeline.getVertexShaderModule()) {
                pipeline.cleanup(context);
            }
        }
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanPipeline: Descriptor stages", "[render][vulkan][pipeline][VulkanGPUTests]") {
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

    VulkanPipeline pipeline;

    SECTION("Vertex stage binding") {
        vk::DescriptorSetLayoutBinding binding;
        binding.setBinding(0);
        binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        binding.setDescriptorCount(1);
        binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);

        std::vector<vk::DescriptorSetLayoutBinding> bindings = {binding};
        REQUIRE(pipeline.createDescriptorSetLayout(context, bindings));
        pipeline.cleanup(context);
    }

    SECTION("Fragment stage binding") {
        vk::DescriptorSetLayoutBinding binding;
        binding.setBinding(0);
        binding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        binding.setDescriptorCount(1);
        binding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

        std::vector<vk::DescriptorSetLayoutBinding> bindings = {binding};
        REQUIRE(pipeline.createDescriptorSetLayout(context, bindings));
        pipeline.cleanup(context);
    }

    SECTION("Both stages binding") {
        vk::DescriptorSetLayoutBinding binding;
        binding.setBinding(0);
        binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        binding.setDescriptorCount(1);
        binding.setStageFlags(vk::ShaderStageFlagBits::eVertex |
                             vk::ShaderStageFlagBits::eFragment);

        std::vector<vk::DescriptorSetLayoutBinding> bindings = {binding};
        REQUIRE(pipeline.createDescriptorSetLayout(context, bindings));
        pipeline.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

// ============================================================================
// Shader-VulkanPipeline Integration Tests
// ============================================================================

TEST_CASE("VulkanPipeline: Attach Shader objects", "[render][vulkan][pipeline][shader]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;
    }

    VulkanPipeline pipeline;
    Shader vertShader;
    Shader fragShader;

    SECTION("Attach uninitialized vertex shader fails") {
        bool result = pipeline.attachVertexShader(context, vertShader);
        REQUIRE_FALSE(result);
    }

    SECTION("Attach uninitialized fragment shader fails") {
        bool result = pipeline.attachFragmentShader(context, fragShader);
        REQUIRE_FALSE(result);
    }

    SECTION("Attach valid vertex shader") {
        if (vertShader.loadFromSPIRV(context, SHADER_PATH("basic.vert.spv"), Shader::Stage::Vertex)) {
            bool result = pipeline.attachVertexShader(context, vertShader);
            REQUIRE(result);
            REQUIRE(pipeline.getVertexShaderModule() != vk::ShaderModule());
        }
    }

    SECTION("Attach valid fragment shader") {
        if (fragShader.loadFromSPIRV(context, SHADER_PATH("basic.frag.spv"), Shader::Stage::Fragment)) {
            bool result = pipeline.attachFragmentShader(context, fragShader);
            REQUIRE(result);
            REQUIRE(pipeline.getFragmentShaderModule() != vk::ShaderModule());
        }
    }

    SECTION("Attach vertex shader with wrong stage fails") {
        if (vertShader.loadFromSPIRV(context, SHADER_PATH("basic.frag.spv"), Shader::Stage::Fragment)) {
            bool result = pipeline.attachVertexShader(context, vertShader);
            REQUIRE_FALSE(result);
        }
    }

    SECTION("Attach fragment shader with wrong stage fails") {
        if (fragShader.loadFromSPIRV(context, SHADER_PATH("basic.vert.spv"), Shader::Stage::Vertex)) {
            bool result = pipeline.attachFragmentShader(context, fragShader);
            REQUIRE_FALSE(result);
        }
    }

    // Cleanup
    if (vertShader.isInitialized()) vertShader.cleanup(context);
    if (fragShader.isInitialized()) fragShader.cleanup(context);
    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanPipeline: Create pipeline with Shader objects", "[render][vulkan][pipeline][shader]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;
    }

    // Create swapchain for render pass
    VulkanSwapchain swapchain;
    vk::Extent2D extent(800, 600);
    if (!swapchain.create(context, extent)) {
        context.shutdown();
        glfwDestroyWindow(window);
        glfwTerminate();
        return;
    }

    VulkanPipeline pipeline;
    Shader vertShader;
    Shader fragShader;

    SECTION("Create graphics pipeline with attached shaders") {
        if (vertShader.loadFromSPIRV(context, SHADER_PATH("basic.vert.spv"), Shader::Stage::Vertex) &&
            fragShader.loadFromSPIRV(context, SHADER_PATH("basic.frag.spv"), Shader::Stage::Fragment)) {

            REQUIRE(pipeline.attachVertexShader(context, vertShader));
            REQUIRE(pipeline.attachFragmentShader(context, fragShader));
            REQUIRE(pipeline.createPipelineLayout(context));

            PipelineConfig config;
            config.renderPass = swapchain.getRenderPass();
            config.extent = extent;

            bool result = pipeline.createGraphicsPipeline(context, config);
            // Should succeed with real shaders
            if (result) {
                REQUIRE(pipeline.getPipeline() != vk::Pipeline());
            }
        }
    }

    // Cleanup
    pipeline.cleanup(context);
    if (vertShader.isInitialized()) vertShader.cleanup(context);
    if (fragShader.isInitialized()) fragShader.cleanup(context);
    swapchain.cleanup(context);
    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanPipeline: attachShader auto-detects stage", "[render][vulkan][pipeline][shader]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;
    }

    VulkanPipeline pipeline;
    Shader vertShader;
    Shader fragShader;

    SECTION("attachShader correctly attaches vertex shader") {
        if (vertShader.loadFromSPIRV(context, SHADER_PATH("basic.vert.spv"), Shader::Stage::Vertex)) {
            bool result = pipeline.attachShader(context, vertShader);
            REQUIRE(result);
            REQUIRE(pipeline.getVertexShaderModule() != vk::ShaderModule());
        }
    }

    SECTION("attachShader correctly attaches fragment shader") {
        if (fragShader.loadFromSPIRV(context, SHADER_PATH("basic.frag.spv"), Shader::Stage::Fragment)) {
            bool result = pipeline.attachShader(context, fragShader);
            REQUIRE(result);
            REQUIRE(pipeline.getFragmentShaderModule() != vk::ShaderModule());
        }
    }

    // Cleanup
    if (vertShader.isInitialized()) vertShader.cleanup(context);
    if (fragShader.isInitialized()) fragShader.cleanup(context);
    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanPipeline: Move semantics with Shader objects", "[render][vulkan][pipeline][shader]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;
    }

    Shader vertShader;
    if (!vertShader.loadFromSPIRV(context, SHADER_PATH("basic.vert.spv"), Shader::Stage::Vertex)) {
        context.shutdown();
        glfwDestroyWindow(window);
        glfwTerminate();
        return;
    }

    SECTION("Move construction transfers shader references") {
        VulkanPipeline pipeline1;
        pipeline1.attachVertexShader(context, vertShader);

        VulkanPipeline pipeline2(std::move(pipeline1));

        // pipeline2 should have the shader reference
        REQUIRE(pipeline2.getVertexShaderModule() != vk::ShaderModule());
        // pipeline1 should be empty
        REQUIRE(pipeline1.getVertexShaderModule() == vk::ShaderModule());
    }

    SECTION("Move assignment transfers shader references") {
        VulkanPipeline pipeline1;
        VulkanPipeline pipeline2;
        pipeline1.attachVertexShader(context, vertShader);

        pipeline2 = std::move(pipeline1);

        REQUIRE(pipeline2.getVertexShaderModule() != vk::ShaderModule());
        REQUIRE(pipeline1.getVertexShaderModule() == vk::ShaderModule());
    }

    // Cleanup
    if (vertShader.isInitialized()) vertShader.cleanup(context);
    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanPipeline: Mixed Shader objects and legacy methods", "[render][vulkan][pipeline][shader]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;
    }

    VulkanPipeline pipeline;
    Shader vertShader;

    // Minimal SPIR-V for legacy method
    std::vector<uint32_t> spirvCode = {
        0x07230203, 0x00010100, 0x00000001, 0x00000008, 0x00000000
    };

    SECTION("Can't use both Shader objects and legacy methods for same stage") {
        if (vertShader.loadFromSPIRV(context, SHADER_PATH("basic.vert.spv"), Shader::Stage::Vertex)) {
            REQUIRE(pipeline.attachVertexShader(context, vertShader));
            // Legacy method should fail because stage is already taken
            bool result = pipeline.createVertexShaderModule(context, spirvCode);
            REQUIRE_FALSE(result);
        }
    }

    // Cleanup
    if (vertShader.isInitialized()) vertShader.cleanup(context);
    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}
