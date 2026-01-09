#pragma once

#include <string>
#include <functional>
#include <imgui.h>
#include <vulkan/vulkan.hpp>

class VulkanContext;

namespace usd_vk {

/**
 * @brief Main UI for USD Vulkan Engine
 *
 * Provides:
 * - Menu bar with File menu
 * - USD file loader dialog
 * - Playback controls (play/pause, frame stepping)
 * - Status information display
 */
class ImGuiUI {
public:
    // Callback types
    using LoadFileCallback = std::function<void(const std::string&)>;
    using PlayPauseCallback = std::function<void()>;
    using StepFrameCallback = std::function<void(int)>;
    using WireframeCallback = std::function<void(bool)>;  // bool = new wireframe state

    ImGuiUI();
    ~ImGuiUI() = default;

    /**
     * @brief Set callbacks for UI actions
     */
    void setLoadFileCallback(LoadFileCallback cb) { m_loadFileCallback = cb; }
    void setPlayPauseCallback(PlayPauseCallback cb) { m_playPauseCallback = cb; }
    void setStepFrameCallback(StepFrameCallback cb) { m_stepFrameCallback = cb; }
    void setWireframeCallback(WireframeCallback cb) { m_wireframeCallback = cb; }

    /**
     * @brief Set wireframe mode state (for UI display)
     */
    void setWireframeMode(bool wireframe) { m_wireframeMode = wireframe; }
    bool isWireframeMode() const { return m_wireframeMode; }

    /**
     * @brief Set current playback state
     */
    void setPlaying(bool playing) { m_playing = playing; }
    bool isPlaying() const { return m_playing; }

    /**
     * @brief Set current frame number
     */
    void setCurrentFrame(float frame) { m_currentFrame = frame; }
    float getCurrentFrame() const { return m_currentFrame; }

    /**
     * @brief Set frame range
     */
    void setFrameRange(float start, float end) {
        m_startFrame = start;
        m_endFrame = end;
    }

    /**
     * @brief Set current USD file path
     */
    void setCurrentFile(const std::string& file) { m_currentFile = file; }
    const std::string& getCurrentFile() const { return m_currentFile; }

    /**
     * @brief Render the main UI
     * Call this each frame after ImGui::NewFrame()
     */
    void render();

    /**
     * @brief Render ImGui windows
     */
    void renderMainMenuBar();
    void renderFileLoader();
    void renderPlaybackControls();
    void renderStats();

    /**
     * @brief Show/hide windows
     */
    void showFileLoader() { m_showFileLoader = true; }
    void showStats(bool show) { m_showStats = show; }

private:
    // Callbacks
    LoadFileCallback m_loadFileCallback;
    PlayPauseCallback m_playPauseCallback;
    StepFrameCallback m_stepFrameCallback;
    WireframeCallback m_wireframeCallback;

    // State
    bool m_playing = false;
    bool m_wireframeMode = false;
    float m_currentFrame = 0.0f;
    float m_startFrame = 0.0f;
    float m_endFrame = 100.0f;
    std::string m_currentFile;

    // Window visibility
    bool m_showFileLoader = false;
    bool m_showStats = true;

    // File dialog state
    char m_filePathBuffer[512] = "";
};

} // namespace usd_vk
