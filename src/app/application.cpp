#include "application.hpp"
#include "render/vulkan_context.hpp"
#include "render/renderer.hpp"
#include "scene/scene.hpp"
#include <spdlog/spdlog.h>
#include <iostream>
#include <cmath>

namespace {
    void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        if (g_application && g_application->getContext()) {
            g_application->getContext()->setFramebufferResized(true);
        }
    }

    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (!g_application) return;

        if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_ESCAPE:
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                    break;
                case GLFW_KEY_SPACE:
                    g_application->togglePlayback();
                    break;
                case GLFW_KEY_LEFT:
                    g_application->previousFrame();
                    break;
                case GLFW_KEY_RIGHT:
                    g_application->nextFrame();
                    break;
            }
        }
    }

    void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
        // Camera rotation could be handled here
    }

    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        // Camera zoom could be handled here
    }
}

Application* g_application = nullptr;

Application::Application() = default;

Application::~Application() {
    destroyWindow();
    m_scene.reset();
    m_renderer.reset();
    m_context.reset();

    if (g_application == this) {
        g_application = nullptr;
    }
}

bool Application::initialize(const std::string& windowTitle, int width, int height) {
    if (m_initialized) {
        SPDLOG_ERROR("Application already initialized");
        return false;
    }

    m_windowWidth = width;
    m_windowHeight = height;

    if (!createWindow(windowTitle, width, height)) {
        return false;
    }

    // Initialize Vulkan context
    m_context = std::make_unique<VulkanContext>();
    if (!m_context->initialize(m_window, true)) {
        SPDLOG_ERROR("Failed to initialize Vulkan context");
        destroyWindow();
        return false;
    }

    // Initialize renderer
    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->initialize(*m_context)) {
        SPDLOG_ERROR("Failed to initialize renderer");
        return false;
    }

    // Initialize scene
    m_scene = std::make_unique<Scene>();

    g_application = this;
    m_initialized = true;
    m_running = true;

    SPDLOG_INFO("Application initialized: {}x{}", width, height);
    return true;
}

bool Application::loadScene(const std::string& filePath) {
    if (!m_scene) {
        SPDLOG_ERROR("Scene not initialized");
        return false;
    }

    SPDLOG_INFO("Loading scene: {}", filePath);
    if (!m_scene->load(filePath)) {
        SPDLOG_ERROR("Failed to load scene: {}", filePath);
        return false;
    }

    // Update renderer with scene data
    if (m_renderer) {
        m_renderer->setScene(m_scene.get());
    }

    return true;
}

void Application::run() {
    if (!m_initialized) {
        SPDLOG_ERROR("Application not initialized");
        return;
    }

    mainLoop();
}

bool Application::createWindow(const std::string& title, int width, int height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        SPDLOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetScrollCallback(m_window, scrollCallback);

    SPDLOG_INFO("Window created: {}x{}", width, height);
    return true;
}

void Application::destroyWindow() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

void Application::mainLoop() {
    m_lastFrameTime = static_cast<float>(glfwGetTime());

    while (m_running && !glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        handleInput();

        float currentTime = static_cast<float>(glfwGetTime());
        m_deltaTime = currentTime - m_lastFrameTime;
        m_lastFrameTime = currentTime;
        m_elapsedTime += m_deltaTime;

        // Update animation time
        if (m_playing) {
            m_currentFrame += m_deltaTime * 24.0f * m_playbackSpeed;  // 24 fps
            if (m_currentFrame > m_endFrame) {
                if (m_loop) {
                    m_currentFrame = m_startFrame;
                } else {
                    m_currentFrame = m_endFrame;
                    m_playing = false;
                }
            }
        }

        // Render
        if (m_renderer && m_context) {
            // Sync current animation time to context (respects pause/play state)
            m_context->setElapsedTime(static_cast<float>(getCurrentTime()));
            m_renderer->render(*m_context, m_scene.get());
        }
    }

    // Wait for device to finish before cleanup
    if (m_context) {
        m_context->getDevice().waitIdle();
    }
}

void Application::handleInput() {
    // Input is now handled via keyCallback for single-press actions
    // This function is kept for future continuous input handling (e.g. camera movement)
}

double Application::getCurrentTime() const {
    if (m_playing) {
        return m_elapsedTime;
    }
    return m_currentFrame / 24.0;  // Convert frame to seconds
}
