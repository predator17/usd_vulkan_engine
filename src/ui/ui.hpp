#pragma once

// UI module - Dear ImGui integration for USD Vulkan Engine

#include "imgui_context.hpp"
#include "imgui_renderer.hpp"
#include "imgui_ui.hpp"
#include "imgui_file_dialog.hpp"

// Convenience using declarations
namespace usd_vk {
    using ImGuiContextPtr = std::unique_ptr<ImGuiContext>;
    using ImGuiRendererPtr = std::unique_ptr<ImGuiRenderer>;
    using ImGuiUIPtr = std::unique_ptr<ImGuiUI>;
}
