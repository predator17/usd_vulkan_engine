#include "imgui_context.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"

#include <spdlog/spdlog.h>

namespace usd_vk {

ImGuiContext::ImGuiContext() = default;

ImGuiContext::~ImGuiContext() {
    shutdown();
}

bool ImGuiContext::initialize(GLFWwindow* window) {
    if (m_initialized) {
        SPDLOG_WARN("ImGuiContext already initialized");
        return true;
    }

    // Create Dear ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    m_io = &ImGui::GetIO();
    m_style = &ImGui::GetStyle();

    // Enable keyboard and gamepad controls
    m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Setup ImGui style
    setDarkTheme();

    // Setup Platform/Renderer backends
    if (!ImGui_ImplGlfw_InitForVulkan(window, true)) {
        SPDLOG_ERROR("Failed to initialize ImGui GLFW backend");
        ImGui::DestroyContext();
        return false;
    }

    // Load default font
    m_io->Fonts->AddFontDefault();

    m_initialized = true;
    SPDLOG_INFO("ImGuiContext initialized");
    return true;
}

void ImGuiContext::shutdown() {
    if (!m_initialized) {
        return;
    }

    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_io = nullptr;
    m_style = nullptr;
    m_initialized = false;

    SPDLOG_INFO("ImGuiContext shutdown");
}

void ImGuiContext::setDarkTheme() {
    if (!m_style) return;
    ImGui::StyleColorsDark();
}

void ImGuiContext::setLightTheme() {
    if (!m_style) return;
    ImGui::StyleColorsLight();
}

void ImGuiContext::newFrame() {
    if (!m_initialized) {
        SPDLOG_ERROR("ImGuiContext::newFrame called before initialization");
        return;
    }

    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiContext::render() {
    if (!m_initialized) {
        return;
    }

    ImGui::Render();
}

} // namespace usd_vk
