#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <memory>

// Forward declarations for ImGui
struct ImGuiIO;
struct ImGuiStyle;
struct ImFontAtlas;

class VulkanContext;

namespace usd_vk {

/**
 * @brief Manages Dear ImGui context and initialization
 *
 * This class handles:
 * - ImGui context creation and destruction
 * - Input handling via GLFW
 * - Font loading
 */
class ImGuiContext {
public:
    ImGuiContext();
    ~ImGuiContext();

    // Non-copyable
    ImGuiContext(const ImGuiContext&) = delete;
    ImGuiContext& operator=(const ImGuiContext&) = delete;

    /**
     * @brief Initialize ImGui context
     * @param window GLFW window for input handling
     * @return true if successful
     */
    bool initialize(GLFWwindow* window);

    /**
     * @brief Shutdown ImGui context
     */
    void shutdown();

    /**
     * @brief Check if initialized
     */
    bool isInitialized() const { return m_initialized; }

    /**
     * @brief Get ImGui IO
     */
    ImGuiIO* getIO() const { return m_io; }

    /**
     * @brief Get ImGui Style
     */
    ImGuiStyle* getStyle() const { return m_style; }

    /**
     * @brief Set dark theme
     */
    void setDarkTheme();

    /**
     * @brief Set light theme
     */
    void setLightTheme();

    /**
     * @brief New frame - call this at the beginning of each frame
     */
    void newFrame();

    /**
     * @brief Render ImGui draw data
     * This should be called after all UI widgets are created
     */
    void render();

private:
    bool m_initialized = false;
    ImGuiIO* m_io = nullptr;
    ImGuiStyle* m_style = nullptr;
};

} // namespace usd_vk
