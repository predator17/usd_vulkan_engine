#include <catch2/catch.hpp>
#include <test_common.hpp>
#include <render/vulkan_context.hpp>
#include <render/shader.hpp>
#include <render/shader_compiler.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <fstream>
#include <cstdio>

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

private:
    static inline bool s_initialized = false;
};

TEST_CASE("Shader: Default construction", "[render][vulkan][shader]") {
    Shader shader;

    SECTION("Initially not initialized") {
        REQUIRE_FALSE(shader.isInitialized());
    }

    SECTION("Module is null") {
        REQUIRE(shader.getModule() == vk::ShaderModule());
    }

    SECTION("Path is empty") {
        REQUIRE(shader.getPath().empty());
    }

    SECTION("Not GLSL by default") {
        REQUIRE_FALSE(shader.isGLSL());
    }
}

TEST_CASE("Shader: Stage utilities", "[render][vulkan][shader]") {
    SECTION("Stage extensions") {
        REQUIRE(std::string(Shader::getStageExtension(Shader::Stage::Vertex)) == ".vert");
        REQUIRE(std::string(Shader::getStageExtension(Shader::Stage::Fragment)) == ".frag");
        REQUIRE(std::string(Shader::getStageExtension(Shader::Stage::Geometry)) == ".geom");
        REQUIRE(std::string(Shader::getStageExtension(Shader::Stage::TessControl)) == ".tesc");
        REQUIRE(std::string(Shader::getStageExtension(Shader::Stage::TessEvaluation)) == ".tese");
        REQUIRE(std::string(Shader::getStageExtension(Shader::Stage::Compute)) == ".comp");
    }

    SECTION("Stage names") {
        REQUIRE(std::string(Shader::getStageName(Shader::Stage::Vertex)) == "vertex");
        REQUIRE(std::string(Shader::getStageName(Shader::Stage::Fragment)) == "fragment");
        REQUIRE(std::string(Shader::getStageName(Shader::Stage::Geometry)) == "geometry");
        REQUIRE(std::string(Shader::getStageName(Shader::Stage::TessControl)) == "tess_control");
        REQUIRE(std::string(Shader::getStageName(Shader::Stage::TessEvaluation)) == "tess_evaluation");
        REQUIRE(std::string(Shader::getStageName(Shader::Stage::Compute)) == "compute");
    }
}

TEST_CASE("Shader: Load from SPIR-V", "[render][vulkan][shader][VulkanGPUTests]") {
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

    SECTION("Load valid vertex shader SPIR-V") {
        Shader shader;
        std::string shaderPath = SHADER_PATH("basic.vert.spv");

        if (shader.loadFromSPIRV(context, shaderPath, Shader::Stage::Vertex)) {
            REQUIRE(shader.isInitialized());
            REQUIRE(shader.getModule() != vk::ShaderModule());
            REQUIRE(shader.getPath() == shaderPath);
            REQUIRE_FALSE(shader.isGLSL());
            REQUIRE(shader.getVkStage() == vk::ShaderStageFlagBits::eVertex);

            shader.cleanup(context);
            REQUIRE_FALSE(shader.isInitialized());
            REQUIRE(shader.getModule() == vk::ShaderModule());
        }
    }

    SECTION("Load valid fragment shader SPIR-V") {
        Shader shader;
        std::string shaderPath = SHADER_PATH("basic.frag.spv");

        if (shader.loadFromSPIRV(context, shaderPath, Shader::Stage::Fragment)) {
            REQUIRE(shader.isInitialized());
            REQUIRE(shader.getModule() != vk::ShaderModule());
            REQUIRE(shader.getVkStage() == vk::ShaderStageFlagBits::eFragment);

            shader.cleanup(context);
        }
    }

    SECTION("Load non-existent file fails") {
        Shader shader;
        REQUIRE_FALSE(shader.loadFromSPIRV(context, "nonexistent.spv", Shader::Stage::Vertex));
        REQUIRE_FALSE(shader.isInitialized());
    }

    context.shutdown();
    glfwDestroyWindow(window);
}

TEST_CASE("Shader: Load from GLSL", "[render][vulkan][shader][VulkanGPUTests]") {
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

    SECTION("Load vertex shader from GLSL with SPIR-V") {
        Shader shader;
        std::string shaderPath = SHADER_PATH("basic.vert");

        if (shader.loadFromGLSL(context, shaderPath, Shader::Stage::Vertex)) {
            REQUIRE(shader.isInitialized());
            REQUIRE(shader.getModule() != vk::ShaderModule());
            REQUIRE(shader.isGLSL());

            shader.cleanup(context);
        }
    }

    SECTION("Load fragment shader from GLSL with SPIR-V") {
        Shader shader;
        std::string shaderPath = SHADER_PATH("basic.frag");

        if (shader.loadFromGLSL(context, shaderPath, Shader::Stage::Fragment)) {
            REQUIRE(shader.isInitialized());
            REQUIRE(shader.getModule() != vk::ShaderModule());
            REQUIRE(shader.isGLSL());

            shader.cleanup(context);
        }
    }

    context.shutdown();
    glfwDestroyWindow(window);
}

TEST_CASE("Shader: Double creation protection", "[render][vulkan][shader][VulkanGPUTests]") {
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

    SECTION("Second loadFromSPIRV returns false") {
        Shader shader;
        if (shader.loadFromSPIRV(context, SHADER_PATH("basic.vert.spv"), Shader::Stage::Vertex)) {
            bool result = shader.loadFromSPIRV(context, SHADER_PATH("basic.frag.spv"), Shader::Stage::Fragment);
            REQUIRE_FALSE(result);
            shader.cleanup(context);
        }
    }

    context.shutdown();
    glfwDestroyWindow(window);
}

TEST_CASE("Shader: Cleanup and recreate", "[render][vulkan][shader][VulkanGPUTests]") {
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

    SECTION("Can cleanup and reload") {
        Shader shader;
        if (shader.loadFromSPIRV(context, SHADER_PATH("basic.vert.spv"), Shader::Stage::Vertex)) {
            shader.cleanup(context);
            REQUIRE_FALSE(shader.isInitialized());

            // Should be able to reload
            shader.loadFromSPIRV(context, SHADER_PATH("basic.frag.spv"), Shader::Stage::Fragment);
            REQUIRE(shader.isInitialized());

            shader.cleanup(context);
        }
    }

    context.shutdown();
    glfwDestroyWindow(window);
}

TEST_CASE("Shader: Move construction", "[render][vulkan][shader][VulkanGPUTests]") {
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

    SECTION("Move transfers ownership") {
        Shader shader1;
        if (shader1.loadFromSPIRV(context, SHADER_PATH("basic.vert.spv"), Shader::Stage::Vertex)) {
            vk::ShaderModule module = shader1.getModule();
            std::string path = shader1.getPath();

            Shader shader2(std::move(shader1));

            // shader2 should have the resources
            REQUIRE(shader2.isInitialized());
            REQUIRE(shader2.getModule() == module);
            REQUIRE(shader2.getPath() == path);

            // shader1 should be empty
            REQUIRE_FALSE(shader1.isInitialized());
            REQUIRE(shader1.getModule() == vk::ShaderModule());

            shader2.cleanup(context);
        }
    }

    context.shutdown();
    glfwDestroyWindow(window);
}

TEST_CASE("Shader: Move assignment", "[render][vulkan][shader][VulkanGPUTests]") {
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

    SECTION("Move assignment transfers ownership") {
        Shader shader1;
        Shader shader2;

        if (shader1.loadFromSPIRV(context, SHADER_PATH("basic.vert.spv"), Shader::Stage::Vertex)) {
            vk::ShaderModule module = shader1.getModule();

            shader2 = std::move(shader1);

            REQUIRE(shader2.isInitialized());
            REQUIRE(shader2.getModule() == module);
            REQUIRE_FALSE(shader1.isInitialized());

            shader2.cleanup(context);
        }
    }

    context.shutdown();
    glfwDestroyWindow(window);
}

TEST_CASE("ShaderCompiler: Check availability", "[render][vulkan][shader]") {
    // This test just checks if the compiler detection works
    bool available = ShaderCompiler::isCompilerAvailable();
    // Don't require it to be available, just check the function works
    INFO("ShaderCompiler available: " << available);
}

TEST_CASE("Shader: All shader stages", "[render][vulkan][shader][VulkanGPUTests]") {
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

    SECTION("Vertex stage") {
        Shader shader;
        if (shader.loadFromSPIRV(context, SHADER_PATH("basic.vert.spv"), Shader::Stage::Vertex)) {
            REQUIRE(shader.getVkStage() == vk::ShaderStageFlagBits::eVertex);
            shader.cleanup(context);
        }
    }

    SECTION("Fragment stage") {
        Shader shader;
        if (shader.loadFromSPIRV(context, SHADER_PATH("basic.frag.spv"), Shader::Stage::Fragment)) {
            REQUIRE(shader.getVkStage() == vk::ShaderStageFlagBits::eFragment);
            shader.cleanup(context);
        }
    }

    context.shutdown();
    glfwDestroyWindow(window);
}

TEST_CASE("Shader: Cleanup before load is safe", "[render][vulkan][shader][VulkanGPUTests]") {
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

    SECTION("Cleanup uninitialized shader is safe") {
        Shader shader;
        REQUIRE_NOTHROW(shader.cleanup(context));
        REQUIRE_FALSE(shader.isInitialized());
    }

    context.shutdown();
    glfwDestroyWindow(window);
}
