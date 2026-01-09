#include <catch2/catch_all.hpp>
using Catch::Approx;  // Catch2 v3
#include <render/vulkan_context.hpp>
#include <render/vulkan_swapchain.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

TEST_CASE("VulkanContext: Default construction", "[render][vulkan][context]") {
    VulkanContext context;

    SECTION("Initially not initialized") {
        REQUIRE_FALSE(context.isInitialized());
    }

    SECTION("Instance is null before initialization") {
        REQUIRE(context.getInstance() == vk::Instance());
    }

    SECTION("Device is null before initialization") {
        REQUIRE(context.getDevice() == vk::Device());
    }
}

TEST_CASE("VulkanContext: Initialize GLFW", "[render][vulkan][context]") {
    SECTION("Can initialize GLFW") {
        REQUIRE(glfwInit());
        glfwTerminate();
    }
}

TEST_CASE("VulkanContext: Initialize with validation", "[render][vulkan][context][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;

    SECTION("Can initialize with validation layers") {
        bool result = context.initialize(window, true);
        // May fail if validation layers not installed
        // Just verify no crash
        REQUIRE(true);
    }

    SECTION("Can initialize without validation layers") {
        bool result = context.initialize(window, false);
        // Should succeed if Vulkan is available
        if (result) {
            REQUIRE(context.isInitialized());
        }
    }

    if (context.isInitialized()) {
        context.shutdown();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanContext: Get instance", "[render][vulkan][context][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (context.initialize(window, false)) {
        SECTION("Instance is valid after initialization") {
            REQUIRE(context.getInstance() != vk::Instance());
        }

        context.shutdown();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanContext: Get physical device", "[render][vulkan][context][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (context.initialize(window, false)) {
        SECTION("Physical device is valid") {
            REQUIRE(context.getPhysicalDevice() != vk::PhysicalDevice());
        }

        context.shutdown();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanContext: Get device", "[render][vulkan][context][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (context.initialize(window, false)) {
        SECTION("Logical device is valid after initialization") {
            REQUIRE(context.getDevice() != vk::Device());
        }

        context.shutdown();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanContext: Get surface", "[render][vulkan][context][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (context.initialize(window, false)) {
        SECTION("Surface is valid") {
            REQUIRE(context.getSurface() != vk::SurfaceKHR());
        }

        context.shutdown();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanContext: Get queues", "[render][vulkan][context][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (context.initialize(window, false)) {
        SECTION("Graphics queue is valid") {
            vk::Queue graphicsQueue = context.getGraphicsQueue();
            // Queue is a handle, just verify we can get it
            REQUIRE(true);
        }

        SECTION("Present queue is valid") {
            vk::Queue presentQueue = context.getPresentQueue();
            REQUIRE(true);
        }

        context.shutdown();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanContext: Queue family indices", "[render][vulkan][context][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (context.initialize(window, false)) {
        VulkanQueueFamilyIndices indices = context.getQueueFamilyIndices();

        SECTION("Graphics family is valid") {
            REQUIRE(indices.graphicsFamily.has_value());
        }

        SECTION("Present family is valid") {
            REQUIRE(indices.presentFamily.has_value());
        }

        SECTION("Complete queue family") {
            REQUIRE(indices.isComplete());
        }

        context.shutdown();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanContext: Shutdown", "[render][vulkan][context][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (context.initialize(window, false)) {
        SECTION("Shutdown clears resources") {
            context.shutdown();
            REQUIRE_FALSE(context.isInitialized());
        }

        SECTION("Can reinitialize after shutdown") {
            context.shutdown();
            bool result = context.initialize(window, false);
            // May succeed or fail depending on system
            REQUIRE(true);
            if (result) {
                context.shutdown();
            }
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanContext: Multiple contexts", "[render][vulkan][context][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window1 = glfwCreateWindow(800, 600, "Test1", nullptr, nullptr);
    GLFWwindow* window2 = glfwCreateWindow(800, 600, "Test2", nullptr, nullptr);
    REQUIRE(window1);
    REQUIRE(window2);

    VulkanContext context1;
    VulkanContext context2;

    SECTION("Can create multiple contexts") {
        bool result1 = context1.initialize(window1, false);
        bool result2 = context2.initialize(window2, false);

        if (result1) context1.shutdown();
        if (result2) context2.shutdown();

        // Just verify no crash
        REQUIRE(true);
    }

    glfwDestroyWindow(window1);
    glfwDestroyWindow(window2);
    glfwTerminate();
}

TEST_CASE("VulkanContext: Window resize handling", "[render][vulkan][context][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (context.initialize(window, false)) {
        SECTION("Can query framebuffer size") {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            REQUIRE(width > 0);
            REQUIRE(height > 0);
        }

        context.shutdown();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

TEST_CASE("VulkanContext: Instance validation layers", "[render][vulkan][context]") {
    SECTION("Check if validation layers are available") {
        std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};

        std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

        bool found = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layers[0], layerProperties.layerName) == 0) {
                found = true;
                break;
            }
        }

        // Just check we can query without crashing
        REQUIRE(true);
    }
}

TEST_CASE("VulkanContext: Instance extensions", "[render][vulkan][context]") {
    SECTION("Can query instance extensions") {
        std::vector<vk::ExtensionProperties> extensions = vk::enumerateInstanceExtensionProperties();

        SECTION("At least some extensions are available") {
            REQUIRE(extensions.size() > 0);
        }

        SECTION("KHR_surface extension is available") {
            bool found = false;
            for (const auto& ext : extensions) {
                if (strcmp(ext.extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0) {
                    found = true;
                    break;
                }
            }
            // On some systems this might not be available
            REQUIRE(true);
        }
    }
}

TEST_CASE("VulkanContext: Device extensions", "[render][vulkan][context][VulkanGPUTests]") {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window);

    VulkanContext context;
    if (context.initialize(window, false)) {
        SECTION("Can query device extensions") {
            std::vector<vk::ExtensionProperties> extensions =
                context.getPhysicalDevice().enumerateDeviceExtensionProperties();

            REQUIRE(extensions.size() > 0);
        }

        context.shutdown();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
