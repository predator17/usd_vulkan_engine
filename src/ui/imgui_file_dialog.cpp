#include "imgui_file_dialog.hpp"

// Include ImGuiFileDialog with proper namespace
#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGuiFileDialog.h"

namespace usd_vk {

void ImGuiFileDialog::OpenModal(
    const char* key,
    const char* title,
    const char* filter,
    const char* path,
    const char* defaultFileName
) {
    IGFD::FileDialogConfig config;
    config.path = path ? path : ".";
    config.fileName = defaultFileName ? defaultFileName : "";
    IGFD::FileDialog::Instance()->OpenDialog(
        key, title, filter, config
    );
}

bool ImGuiFileDialog::IsDone(const char* key) {
    return !IGFD::FileDialog::Instance()->IsOpened(key);
}

bool ImGuiFileDialog::HasResult() {
    return !IGFD::FileDialog::Instance()->GetSelection().empty();
}

std::string ImGuiFileDialog::GetResultName() {
    const auto& selection = IGFD::FileDialog::Instance()->GetSelection();
    if (!selection.empty()) {
        return selection.begin()->second;
    }
    return "";
}

void ImGuiFileDialog::Close() {
    IGFD::FileDialog::Instance()->Close();
}

} // namespace usd_vk
