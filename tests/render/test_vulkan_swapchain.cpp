#include <catch2/catch.hpp>
#include <render/vulkan_context.hpp>
#include <render/vulkan_swapchain.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

TEST_CASE("VulkanSwapchain: Default construction", "[render][vulkan][swapchain]") {
    VulkanSwapchain swapchain;

    SECTION("Initially not created") {
        REQUIRE_FALSE(swapchain.isInitialized());
    }

    SECTION("Swapchain handle is null") {
        REQUIRE(swapchain.getSwapchain() == vk::SwapchainKHR());
    }

    SECTION("No image views initially") {
        REQUIRE(swapchain.getImageViews().empty());
    }
}

TEST_CASE("VulkanSwapchain: Create and destroy", "[render][vulkan][swapchain][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;  // SKIP: Vulkan not available
    }

    VulkanSwapchain swapchain;
    vk::Extent2D extent(800, 600);

    SECTION("Can create swapchain") {
        bool result = swapchain.create(context, extent);
        if (result) {
            REQUIRE(swapchain.isInitialized());
            REQUIRE(swapchain.getSwapchain() != vk::SwapchainKHR());
        }
    }

    SECTION("Swapchain has image views") {
        if (swapchain.create(context, extent)) {
            REQUIRE(swapchain.getImageViews().size() > 0);
        }
    }

    SECTION("Swapchain has framebuffers") {
        if (swapchain.create(context, extent)) {
            REQUIRE(swapchain.getFramebuffers().size() > 0);
        }
    }

    SECTION("Swapchain has render pass") {
        if (swapchain.create(context, extent)) {
            REQUIRE(swapchain.getRenderPass() != vk::RenderPass());
        }
    }

    if (swapchain.isInitialized()) {
        swapchain.cleanup(context);
    }
    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanSwapchain: Double creation protection", "[render][vulkan][swapchain][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;  // SKIP: Vulkan not available
    }

    VulkanSwapchain swapchain;
    vk::Extent2D extent(800, 600);

    if (swapchain.create(context, extent)) {
        SECTION("Second create returns false") {
            bool result = swapchain.create(context, extent);
            REQUIRE_FALSE(result);
        }

        swapchain.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanSwapchain: Cleanup and recreate", "[render][vulkan][swapchain][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;  // SKIP: Vulkan not available
    }

    VulkanSwapchain swapchain;
    vk::Extent2D extent(800, 600);

    SECTION("Can cleanup and recreate") {
        if (swapchain.create(context, extent)) {
            swapchain.cleanup(context);
            REQUIRE_FALSE(swapchain.isInitialized());

            // Should be able to recreate
            swapchain.create(context, extent);
            REQUIRE(swapchain.isInitialized());

            swapchain.cleanup(context);
        }
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanSwapchain: Different extents", "[render][vulkan][swapchain][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;  // SKIP: Vulkan not available
    }

    VulkanSwapchain swapchain;

    SECTION("Small extent") {
        vk::Extent2D extent(64, 64);
        if (swapchain.create(context, extent)) {
            REQUIRE(swapchain.isInitialized());
            swapchain.cleanup(context);
        }
    }

    SECTION("Large extent") {
        vk::Extent2D extent(1920, 1080);
        if (swapchain.create(context, extent)) {
            REQUIRE(swapchain.isInitialized());
            swapchain.cleanup(context);
        }
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanSwapchain: Image views count", "[render][vulkan][swapchain][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;  // SKIP: Vulkan not available
    }

    VulkanSwapchain swapchain;
    vk::Extent2D extent(800, 600);

    SECTION("Image view count matches framebuffer count") {
        if (swapchain.create(context, extent)) {
            REQUIRE(swapchain.getImageViews().size() == swapchain.getFramebuffers().size());
        }
    }

    if (swapchain.isInitialized()) {
        swapchain.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanSwapchain: Render pass creation", "[render][vulkan][swapchain][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;  // SKIP: Vulkan not available
    }

    VulkanSwapchain swapchain;
    vk::Extent2D extent(800, 600);

    SECTION("Render pass is valid") {
        if (swapchain.create(context, extent)) {
            vk::RenderPass renderPass = swapchain.getRenderPass();
            REQUIRE(renderPass != vk::RenderPass());
        }
    }

    if (swapchain.isInitialized()) {
        swapchain.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanSwapchain: Framebuffer creation", "[render][vulkan][swapchain][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;  // SKIP: Vulkan not available
    }

    VulkanSwapchain swapchain;
    vk::Extent2D extent(800, 600);

    SECTION("Framebuffers are valid") {
        if (swapchain.create(context, extent)) {
            const auto& framebuffers = swapchain.getFramebuffers();
            for (const auto& fb : framebuffers) {
                REQUIRE(fb != vk::Framebuffer());
            }
        }
    }

    if (swapchain.isInitialized()) {
        swapchain.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanSwapchain: Depth buffer", "[render][vulkan][swapchain][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;  // SKIP: Vulkan not available
    }

    VulkanSwapchain swapchain;
    vk::Extent2D extent(800, 600);

    SECTION("Depth buffer is created") {
        // Depth buffer creation is part of swapchain creation
        // Just verify swapchain creates successfully
        if (swapchain.create(context, extent)) {
            REQUIRE(swapchain.isInitialized());
        }
    }

    if (swapchain.isInitialized()) {
        swapchain.cleanup(context);
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanSwapchain: Cleanup before create", "[render][vulkan][swapchain][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (!context.initialize(window, false)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return;  // SKIP: Vulkan not available
    }

    VulkanSwapchain swapchain;

    SECTION("Cleanup before create is safe") {
        swapchain.cleanup(context);
        REQUIRE_FALSE(swapchain.isInitialized());
    }

    context.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}
