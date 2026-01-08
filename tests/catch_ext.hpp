#pragma once

// Extended Catch2 macros and utilities for USD Vulkan Engine testing
#include <catch2/catch.hpp>
#include <vulkan/vulkan.hpp>
#include <string>
#include <sstream>

// Custom macro to check Vulkan results
#define VK_REQUIRE(result) REQUIRE((result) == vk::Result::eSuccess)
#define VK_CHECK(result) CHECK((result) == vk::Result::eSuccess)

// Helper to convert Vulkan handle to string for debugging
template<typename T>
std::string vkHandleToString(const T& handle) {
    std::ostringstream oss;
    oss << reinterpret_cast<const void*>(static_cast<const typename T::NativeType&>(handle));
    return oss.str();
}

// Custom matcher for approximately equal vec3
struct Vec3Approx {
    glm::vec3 value;
    float epsilon;

    Vec3Approx(glm::vec3 v, float eps = 0.0001f) : value(v), epsilon(eps) {}
};

inline bool operator==(const glm::vec3& lhs, const Vec3Approx& rhs) {
    return std::abs(lhs.x - rhs.value.x) < rhs.epsilon &&
           std::abs(lhs.y - rhs.value.y) < rhs.epsilon &&
           std::abs(lhs.z - rhs.value.z) < rhs.epsilon;
}

// Custom matcher for approximately equal mat4 (simplified - checks diagonal)
struct Mat4Approx {
    glm::mat4 value;
    float epsilon;

    Mat4Approx(glm::mat4 m, float eps = 0.0001f) : value(m), epsilon(eps) {}
};

inline bool operator==(const glm::mat4& lhs, const Mat4Approx& rhs) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (std::abs(lhs[i][j] - rhs.value[i][j]) >= rhs.epsilon) {
                return false;
            }
        }
    }
    return true;
}

// String makers for better test output
namespace Catch {
    template<>
    struct StringMaker<vk::Result> {
        static std::string convert(vk::Result result) {
            switch (result) {
                case vk::Result::eSuccess: return "vk::Result::eSuccess";
                case vk::Result::eNotReady: return "vk::Result::eNotReady";
                case vk::Result::eTimeout: return "vk::Result::eTimeout";
                case vk::Result::eErrorOutOfHostMemory: return "vk::Result::eErrorOutOfHostMemory";
                case vk::Result::eErrorOutOfDeviceMemory: return "vk::Result::eErrorOutOfDeviceMemory";
                case vk::Result::eErrorInitializationFailed: return "vk::Result::eErrorInitializationFailed";
                case vk::Result::eErrorDeviceLost: return "vk::Result::eErrorDeviceLost";
                case vk::Result::eErrorLayerNotPresent: return "vk::Result::eErrorLayerNotPresent";
                case vk::Result::eErrorExtensionNotPresent: return "vk::Result::eErrorExtensionNotPresent";
                case vk::Result::eErrorFeatureNotPresent: return "vk::Result::eErrorFeatureNotPresent";
                default: return "vk::Result::Unknown(" + std::to_string(static_cast<int>(result)) + ")";
            }
        }
    };

    template<>
    struct StringMaker<glm::vec3> {
        static std::string convert(const glm::vec3& v) {
            std::ostringstream oss;
            oss << "(" << v.x << ", " << v.y << ", " << v.z << ")";
            return oss.str();
        }
    };

    template<>
    struct StringMaker<glm::mat4> {
        static std::string convert(const glm::mat4& m) {
            std::ostringstream oss;
            oss << "mat4(\n";
            for (int i = 0; i < 4; ++i) {
                oss << "  ";
                for (int j = 0; j < 4; ++j) {
                    oss << m[i][j];
                    if (j < 3) oss << ", ";
                }
                if (i < 3) oss << "\n";
            }
            oss << ")";
            return oss.str();
        }
    };
}

// Tag for Vulkan-specific tests that require a GPU
struct VulkanGPUTests {};

// Tag for slow tests that should be skipped in quick test runs
struct SlowTests {};

// Tag for USD file I/O tests
struct USDFileTests {};

// Helper section to skip GPU tests if no Vulkan device is available
#define SKIP_IF_NO_VULKAN_DEVICE() \
    do { \
        std::vector<vk::PhysicalDevice> devices = vk::enumeratePhysicalDevices(instance); \
        if (devices.empty()) { \
            SKIP("No Vulkan physical devices available"); \
        } \
    } while(0)

// Helper to skip tests if validation layers are not available
#define SKIP_IF_NO_VALIDATION_LAYERS() \
    do { \
        std::vector<vk::LayerProperties> layers = vk::enumerateInstanceLayerProperties(); \
        bool found = false; \
        for (const auto& layer : layers) { \
            if (std::string(layer.layerName) == "VK_LAYER_KHRONOS_validation") { \
                found = true; \
                break; \
            } \
        } \
        if (!found) { \
            SKIP("Validation layers not available"); \
        } \
    } while(0)
