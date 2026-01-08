#pragma once

#include <string>
#include <memory>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

class VulkanContext;
class Renderer;
class Scene;

namespace usd_vk {
    class ImGuiContext;
    class ImGuiRenderer;
    class ImGuiUI;
}

/**
 * @brief Main application class for USD Vulkan Engine
 *
 * Manages window creation, main loop, and application lifecycle.
 */
class Application {
public:
    Application();
    ~Application();

    // Non-copyable
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /**
     * @brief Initialize the application
     * @param windowTitle Title for the window
     * @param width Window width
     * @param height Window height
     * @return true if successful
     */
    bool initialize(const std::string& windowTitle, int width, int height);

    /**
     * @brief Load a USD scene from file
     * @param filePath Path to .usda/.usdc/.usd file
     * @return true if successful
     */
    bool loadScene(const std::string& filePath);

    /**
     * @brief Run the main render loop
     */
    void run();

    /**
     * @brief Request application shutdown
     */
    void requestShutdown() { m_running = false; }

    /**
     * @brief Check if application is running
     */
    bool isRunning() const { return m_running; }

    /**
     * @brief Get the GLFW window
     */
    GLFWwindow* getWindow() const { return m_window; }

    /**
     * @brief Get the Vulkan context
     */
    VulkanContext* getContext() const { return m_context.get(); }

    /**
     * @brief Get the renderer
     */
    Renderer* getRenderer() const { return m_renderer.get(); }

    /**
     * @brief Get the current scene
     */
    Scene* getScene() const { return m_scene.get(); }

    /**
     * @brief Get current frame time
     */
    float getDeltaTime() const { return m_deltaTime; }

    /**
     * @brief Get total elapsed time
     */
    float getElapsedTime() const { return m_elapsedTime; }

    /**
     * @brief Get current time in seconds for USD animation
     */
    double getCurrentTime() const;

    /**
     * @brief Set animation playback speed
     * @param speed Multiplier (1.0 = normal speed)
     */
    void setPlaybackSpeed(float speed) { m_playbackSpeed = speed; }

    /**
     * @brief Get animation playback speed
     */
    float getPlaybackSpeed() const { return m_playbackSpeed; }

    /**
     * @brief Toggle animation playback
     */
    void togglePlayback() { m_playing = !m_playing; SPDLOG_INFO("Animation {}", m_playing ? "playing" : "paused"); }

    /**
     * @brief Step to previous frame
     */
    void previousFrame() { m_currentFrame = std::max(m_startFrame, m_currentFrame - 1.0f); m_playing = false; SPDLOG_INFO("Frame: {}", m_currentFrame); }

    /**
     * @brief Step to next frame
     */
    void nextFrame() { m_currentFrame = std::min(m_endFrame, m_currentFrame + 1.0f); m_playing = false; SPDLOG_INFO("Frame: {}", m_currentFrame); }

private:
    bool createWindow(const std::string& title, int width, int height);
    void destroyWindow();
    void mainLoop();
    void handleInput();
    void setupUICallbacks();

    std::unique_ptr<VulkanContext> m_context;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Scene> m_scene;

    // ImGui components
    std::unique_ptr<usd_vk::ImGuiContext> m_imguiContext;
    std::unique_ptr<usd_vk::ImGuiRenderer> m_imguiRenderer;
    std::unique_ptr<usd_vk::ImGuiUI> m_imguiUI;

    GLFWwindow* m_window = nullptr;
    int m_windowWidth = 1280;
    int m_windowHeight = 720;

    bool m_running = false;
    bool m_initialized = false;

    // Timing
    float m_deltaTime = 0.0f;
    float m_elapsedTime = 0.0f;
    float m_lastFrameTime = 0.0f;
    float m_playbackSpeed = 1.0f;

    // Animation control
    bool m_playing = true;
    float m_currentFrame = 0.0f;
    float m_startFrame = 0.0f;
    float m_endFrame = 100.0f;
    bool m_loop = true;
};

// Global application pointer for GLFW callbacks
extern Application* g_application;
