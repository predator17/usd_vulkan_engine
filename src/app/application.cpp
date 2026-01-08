#include "application.hpp"
#include "render/vulkan_context.hpp"
#include "render/renderer.hpp"
#include "scene/scene.hpp"
#include "camera/camera.hpp"
#include "ui/imgui_context.hpp"
#include "ui/imgui_renderer.hpp"
#include "ui/imgui_ui.hpp"
#include "imgui.h"
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

        // Check if ImGui wants to capture keyboard input
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard) {
            return;
        }

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
        if (!g_application) return;

        // Check if ImGui wants to capture mouse input
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            return;
        }

        g_application->handleMouseMotion(xpos, ypos);
    }

    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        if (!g_application) return;

        // Check if ImGui wants to capture mouse input
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            return;
        }

        g_application->handleMouseScroll(yoffset);
    }

    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        if (!g_application) return;

        // Check if ImGui wants to capture mouse input
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            return;
        }

        g_application->handleMouseButton(button, action, mods);
    }
}

Application* g_application = nullptr;

Application::Application() = default;

Application::~Application() {
    // Shutdown ImGui first (before Vulkan context)
    if (m_imguiRenderer) {
        m_imguiRenderer->shutdown(*m_context);
        m_imguiRenderer.reset();
    }
    if (m_imguiContext) {
        m_imguiContext->shutdown();
        m_imguiContext.reset();
    }
    if (m_imguiUI) {
        m_imguiUI.reset();
    }

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

    // Initialize camera
    m_camera = std::make_unique<Camera>();
    m_renderer->setCamera(m_camera.get());

    SPDLOG_INFO("About to initialize ImGui context...");

    // Initialize ImGui context
    m_imguiContext = std::make_unique<usd_vk::ImGuiContext>();
    if (!m_imguiContext->initialize(m_window)) {
        SPDLOG_ERROR("Failed to initialize ImGui context");
        return false;
    }

    SPDLOG_INFO("ImGui context initialized, about to initialize ImGui renderer...");

    // Initialize ImGui renderer
    m_imguiRenderer = std::make_unique<usd_vk::ImGuiRenderer>();
    if (!m_imguiRenderer->initialize(*m_context)) {
        SPDLOG_ERROR("Failed to initialize ImGui renderer");
        return false;
    }

    SPDLOG_INFO("ImGui renderer initialized, setting up callbacks...");

    // Set ImGui renderer on the main renderer
    m_renderer->setImGuiRenderer(m_imguiRenderer.get());

    // Initialize ImGui UI
    m_imguiUI = std::make_unique<usd_vk::ImGuiUI>();
    setupUICallbacks();

    SPDLOG_INFO("ImGui UI initialized...");

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

    // Update UI with loaded file
    if (m_imguiUI) {
        m_imguiUI->setCurrentFile(filePath);
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
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);

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

        // Update ImGui with current state
        if (m_imguiUI) {
            m_imguiUI->setPlaying(m_playing);
            m_imguiUI->setCurrentFrame(m_currentFrame);
            // Sync wireframe mode state with UI
            if (m_renderer) {
                m_imguiUI->setWireframeMode(m_renderer->isWireframeMode());
            }
        }

        // Start ImGui frame
        if (m_imguiContext) {
            m_imguiContext->newFrame();
        }
        if (m_imguiRenderer) {
            m_imguiRenderer->newFrame();
        }

        // Build ImGui UI
        if (m_imguiUI) {
            m_imguiUI->render();
        }

        // Render scene and ImGui (ImGui draw data is recorded in the render pass)
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

void Application::setupUICallbacks() {
    if (!m_imguiUI) {
        return;
    }

    // Set up load file callback
    m_imguiUI->setLoadFileCallback([this](const std::string& path) {
        loadScene(path);
    });

    // Set up play/pause callback
    m_imguiUI->setPlayPauseCallback([this]() {
        togglePlayback();
    });

    // Set up frame step callback (handles both previous/next and seek)
    m_imguiUI->setStepFrameCallback([this](int delta) {
        if (delta < 0) {
            for (int i = 0; i < -delta; i++) {
                previousFrame();
            }
        } else if (delta > 0) {
            for (int i = 0; i < delta; i++) {
                nextFrame();
            }
        }
    });

    // Set up wireframe callback
    m_imguiUI->setWireframeCallback([this](bool wireframe) {
        if (m_renderer) {
            m_renderer->setWireframeMode(wireframe);
            SPDLOG_INFO("Wireframe mode: {}", wireframe ? "enabled" : "disabled");
        }
    });

    // Set frame range for UI
    m_imguiUI->setFrameRange(m_startFrame, m_endFrame);
}

void Application::handleMouseButton(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            m_mouseLeftPressed = true;
        } else if (action == GLFW_RELEASE) {
            m_mouseLeftPressed = false;
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            m_mouseRightPressed = true;
        } else if (action == GLFW_RELEASE) {
            m_mouseRightPressed = false;
        }
    }
}

void Application::handleMouseMotion(double xpos, double ypos) {
    if (!m_camera) return;

    double deltaX = xpos - m_lastMouseX;
    double deltaY = ypos - m_lastMouseY;

    m_lastMouseX = xpos;
    m_lastMouseY = ypos;

    // Orbit controls with left mouse button
    if (m_mouseLeftPressed) {
        const float orbitSpeed = 0.005f;
        m_camera->orbit(
            static_cast<float>(-deltaX) * orbitSpeed,
            static_cast<float>(-deltaY) * orbitSpeed
        );
    }

    // Pan controls with right mouse button
    if (m_mouseRightPressed) {
        m_camera->pan(
            static_cast<float>(deltaX),
            static_cast<float>(deltaY)
        );
    }
}

void Application::handleMouseScroll(double yoffset) {
    if (!m_camera) return;

    // Zoom with scroll wheel
    const float zoomSpeed = 0.1f;
    m_camera->zoom(static_cast<float>(yoffset) * zoomSpeed);
}
