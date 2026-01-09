#pragma once

#include <string>

// Include ImGuiFileDialog
#include "ImGuiFileDialog.h"

namespace usd_vk {

/**
 * @brief Wrapper around ImGuiFileDialog for file selection
 *
 * Provides a simple static interface to the ImGuiFileDialog library
 */
class ImGuiFileDialog {
public:
    /**
     * @brief Open a modal file dialog
     * @param key Unique identifier for this dialog
     * @param title Dialog title
     * @param filter File filter (e.g., ".usda,.usdc,.usd")
     * @param path Starting directory
     * @param defaultFileName Default file name
     */
    static void OpenModal(
        const char* key,
        const char* title,
        const char* filter,
        const char* path,
        const char* defaultFileName = ""
    );

    /**
     * @brief Check if dialog is done
     * @param key Dialog identifier
     */
    static bool IsDone(const char* key);

    /**
     * @brief Check if a file was selected
     */
    static bool HasResult();

    /**
     * @brief Get the selected file path
     */
    static std::string GetResultName();

    /**
     * @brief Close the dialog
     */
    static void Close();
};

} // namespace usd_vk
