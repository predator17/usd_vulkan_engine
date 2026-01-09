#include "imgui_ui.hpp"

#include "imgui.h"
#include "imgui_file_dialog.hpp"

#include <spdlog/spdlog.h>
#include <cstring>

namespace usd_vk {

ImGuiUI::ImGuiUI() {
    // Initialize file path buffer
    std::strncpy(m_filePathBuffer, m_currentFile.c_str(), sizeof(m_filePathBuffer) - 1);
}

void ImGuiUI::render() {
    renderMainMenuBar();
    renderPlaybackControls();
    if (m_showStats) {
        renderStats();
    }
    if (m_showFileLoader) {
        renderFileLoader();
    }

    // Generate draw data for rendering
    ImGui::Render();
}

void ImGuiUI::renderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open USD...", "Ctrl+O")) {
                m_showFileLoader = true;
            }
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                // Let the window handle this
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::Checkbox("Show Stats", &m_showStats);
            ImGui::Separator();
            if (ImGui::Checkbox("Wireframe Mode", &m_wireframeMode)) {
                if (m_wireframeCallback) {
                    m_wireframeCallback(m_wireframeMode);
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                m_showStats = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void ImGuiUI::renderFileLoader() {
    if (!m_showFileLoader) return;

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoScrollWithMouse;

    if (ImGui::Begin("Open USD File", &m_showFileLoader, windowFlags)) {
        ImGui::Text("Select a USD file to load:");
        ImGui::Spacing();

        // File path input
        ImGui::Text("File Path:");
        ImGui::SameLine();
        if (ImGui::Button("Browse...")) {
            // Open file dialog
            IGFD::FileDialogConfig config;
            config.path = ".";
            IGFD::FileDialog::Instance()->OpenDialog(
                "ChooseUSDFile",
                "Choose USD File",
                ".usda,.usdc,.usd",
                config
            );
        }
        ImGui::SameLine();

        // Input text box
        ImGui::InputText("##FilePath", m_filePathBuffer,
                         sizeof(m_filePathBuffer), ImGuiInputTextFlags_None);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Recent files (placeholder)
        ImGui::Text("Recent Files:");
        if (ImGui::BeginChild("RecentFiles", ImVec2(0, 150))) {
            ImGui::TextDisabled("No recent files");
            if (!m_currentFile.empty()) {
                if (ImGui::Selectable(m_currentFile.c_str())) {
                    std::strncpy(m_filePathBuffer, m_currentFile.c_str(),
                                 sizeof(m_filePathBuffer) - 1);
                }
            }
        }
        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Buttons
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 200);
        if (ImGui::Button("Load", ImVec2(80, 0))) {
            std::string path(m_filePathBuffer);
            if (!path.empty() && m_loadFileCallback) {
                m_currentFile = path;
                m_loadFileCallback(path);
                m_showFileLoader = false;
                SPDLOG_INFO("Loading USD file: {}", path);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0))) {
            m_showFileLoader = false;
        }
    }
    ImGui::End();

    // Render file dialog
    IGFD::FileDialog::Instance()->Display("ChooseUSDFile");

    // Handle file dialog result
    if (!IGFD::FileDialog::Instance()->IsOpened("ChooseUSDFile")) {
        const auto& selection = IGFD::FileDialog::Instance()->GetSelection();
        if (!selection.empty()) {
            std::string path = selection.begin()->second;
            std::strncpy(m_filePathBuffer, path.c_str(), sizeof(m_filePathBuffer) - 1);
        }
    }
}

void ImGuiUI::renderPlaybackControls() {
    // Floating control panel at the bottom
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
                                    viewport->WorkPos.y + viewport->WorkSize.y - 60),
                               ImGuiCond_FirstUseEver, ImVec2(0.5f, 1.0f));
    ImGui::SetNextWindowSize(ImVec2(400, 50), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoScrollWithMouse |
                                   ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoBackground;

    if (ImGui::Begin("PlaybackControls", nullptr, windowFlags)) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));

        // Play/Pause button
        ImVec2 buttonSize(40, 30);
        const char* playIcon = m_playing ? "||" : ">";
        if (ImGui::Button(playIcon, buttonSize)) {
            m_playing = !m_playing;
            if (m_playPauseCallback) {
                m_playPauseCallback();
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(m_playing ? "Pause (Space)" : "Play (Space)");
        }

        ImGui::SameLine();

        // Previous frame button
        if (ImGui::Button("<<", buttonSize)) {
            m_currentFrame = std::max(m_startFrame, m_currentFrame - 1.0f);
            if (m_stepFrameCallback) {
                m_stepFrameCallback(-1);
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Previous Frame (Left)");
        }

        ImGui::SameLine();

        // Frame slider
        ImGui::PushItemWidth(200);
        float prevFrame = m_currentFrame;
        if (ImGui::SliderFloat("##FrameSlider", &m_currentFrame, m_startFrame, m_endFrame,
                                "Frame %.0f")) {
            if (m_stepFrameCallback) {
                int delta = static_cast<int>(m_currentFrame - prevFrame);
                if (delta != 0) {
                    m_stepFrameCallback(delta);
                }
            }
        }
        ImGui::PopItemWidth();

        ImGui::SameLine();

        // Next frame button
        if (ImGui::Button(">>", buttonSize)) {
            m_currentFrame = std::min(m_endFrame, m_currentFrame + 1.0f);
            if (m_stepFrameCallback) {
                m_stepFrameCallback(1);
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Next Frame (Right)");
        }

        ImGui::PopStyleColor();
    }
    ImGui::End();
}

void ImGuiUI::renderStats() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 10,
                                    viewport->WorkPos.y + 40),
                               ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 150), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoBackground;

    if (ImGui::Begin("Stats", &m_showStats, windowFlags)) {
        ImGui::Text("USD Vulkan Engine");
        ImGui::Separator();
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame: %.0f / %.0f", m_currentFrame, m_endFrame);
        ImGui::Text("Status: %s", m_playing ? "Playing" : "Paused");
        ImGui::Text("Display: %s", m_wireframeMode ? "Wireframe" : "Solid");

        if (!m_currentFile.empty()) {
            ImGui::Separator();
            ImGui::Text("File:");
            ImGui::SameLine();
            std::string shortFile = m_currentFile;
            if (shortFile.length() > 30) {
                shortFile = "..." + shortFile.substr(shortFile.length() - 27);
            }
            ImGui::Text("%s", shortFile.c_str());
        }
    }
    ImGui::End();
}

} // namespace usd_vk
