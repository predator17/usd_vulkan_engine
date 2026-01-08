#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>

namespace test_helpers {

// Simple vertex structure for testing
struct Vertex {
    float position[3];
    float normal[3];
    float uv[2];

    Vertex() = default;
    Vertex(float x, float y, float z) : position{x, y, z}, normal{0, 1, 0}, uv{0, 0} {}
};

// Create a simple triangle for testing
inline std::vector<Vertex> createTriangleVertices() {
    return {
        Vertex{0.0f, -0.5f, 0.0f},
        Vertex{0.5f, 0.5f, 0.0f},
        Vertex{-0.5f, 0.5f, 0.0f}
    };
}

// Create a quad for testing
inline std::vector<Vertex> createQuadVertices() {
    return {
        Vertex{-0.5f, -0.5f, 0.0f},
        Vertex{0.5f, -0.5f, 0.0f},
        Vertex{0.5f, 0.5f, 0.0f},
        Vertex{-0.5f, 0.5f, 0.0f}
    };
}

// Create simple indices for a quad (two triangles)
inline std::vector<uint32_t> createQuadIndices() {
    return {0, 1, 2, 2, 3, 0};
}

// Simple MVP matrix struct for uniform buffers
struct MVPUniform {
    float model[16];
    float view[16];
    float projection[16];

    MVPUniform() {
        std::memset(model, 0, sizeof(model));
        std::memset(view, 0, sizeof(view));
        std::memset(projection, 0, sizeof(projection));
        // Identity matrix for model
        model[0] = model[5] = model[10] = model[15] = 1.0f;
        // Simple view matrix (looking at origin)
        view[0] = view[5] = view[10] = view[15] = 1.0f;
        view[14] = -2.0f;  // Move back 2 units
        // Simple perspective-like matrix
        projection[0] = projection[5] = 1.0f;
        projection[10] = -1.0f;
        projection[14] = -1.0f;
        projection[15] = 0.0f;
    }
};

// Minimal SPIR-V shader for testing (valid but simple)
// This is a placeholder - real tests would use pre-compiled SPIR-V files
inline std::vector<uint32_t> createMinimalVertexShaderSPIRV() {
    // This is a minimal valid SPIR-V header (magic number + version)
    // In real implementation, this would load from .spv files
    return {
        0x07230203,  // Magic number
        0x00010100,  // Version 1.0
        0x00000001,  // Generator magic
        0x00000008,  // Bound
        0x00000000   // Schema
    };
}

inline std::vector<uint32_t> createMinimalFragmentShaderSPIRV() {
    // Same minimal header for fragment shader
    return {
        0x07230203,  // Magic number
        0x00010100,  // Version 1.0
        0x00000001,  // Generator magic
        0x00000008,  // Bound
        0x00000000   // Schema
    };
}

// Read SPIR-V file from disk (used when shaders are compiled)
inline std::vector<uint32_t> readSPIRVFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint32_t> buffer(size / sizeof(uint32_t));
    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return buffer;
    }
    return {};
}

// Check if validation layers are available
inline bool checkValidationLayerSupport() {
    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

    for (const char* layerName : {"VK_LAYER_KHRONOS_validation"}) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    return true;
}

// Find queue family indices
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

inline QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
    QueueFamilyIndices indices;

    std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }

        if (device.getSurfaceSupportKHR(i, surface)) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
        i++;
    }

    return indices;
}

// Check device extension support
inline bool checkDeviceExtensionSupport(vk::PhysicalDevice device, const std::vector<const char*>& extensions) {
    std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

// Simple file writing helper for test assets
inline bool writeTestFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    file << content;
    return true;
}

// Print Vulkan error message
inline std::string vkResultToString(vk::Result result) {
    switch (result) {
        case vk::Result::eSuccess: return "eSuccess";
        case vk::Result::eNotReady: return "eNotReady";
        case vk::Result::eTimeout: return "eTimeout";
        case vk::Result::eErrorOutOfHostMemory: return "eErrorOutOfHostMemory";
        case vk::Result::eErrorOutOfDeviceMemory: return "eErrorOutOfDeviceMemory";
        case vk::Result::eErrorInitializationFailed: return "eErrorInitializationFailed";
        case vk::Result::eErrorDeviceLost: return "eErrorDeviceLost";
        case vk::Result::eErrorOutOfDeviceMemory: return "eErrorOutOfDeviceMemory";
        case vk::Result::eErrorOutOfPoolMemory: return "eErrorOutOfPoolMemory";
        default: return "Unknown";
    }
}

} // namespace test_helpers
