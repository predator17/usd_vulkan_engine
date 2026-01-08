# UI library - Dear ImGui integration

set(UI_SOURCES
    imgui_context.cpp
    imgui_renderer.cpp
    imgui_ui.cpp
    imgui_file_dialog.cpp
)

set(UI_HEADERS
    imgui_context.hpp
    imgui_renderer.hpp
    imgui_ui.hpp
    imgui_file_dialog.hpp
)

# ImGui sources from external/
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)
set(IMGUI_HEADERS
    ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/imgui_internal.h
    ${IMGUI_DIR}/imgui_demo.h
    ${IMGUI_DIR}/imstb_rectpack.h
    ${IMGUI_DIR}/imstb_textedit.h
    ${IMGUI_DIR}/imstb_truetype.h
)

# ImGui backends
set(IMGUI_BACKEND_DIR ${IMGUI_DIR}/backends)
set(IMGUI_BACKEND_SOURCES
    ${IMGUI_BACKEND_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_BACKEND_DIR}/imgui_impl_vulkan.cpp
)
set(IMGUI_BACKEND_HEADERS
    ${IMGUI_DIR}/imgui_impl_glfw.h
    ${IMGUI_DIR}/imgui_impl_vulkan.h
    ${IMGUI_DIR}/backends/imgui_impl_glfw.h
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.h
)

# ImGuiFileDialog
set(IMGUI_FILE_DIALOG_DIR ${CMAKE_SOURCE_DIR}/external/ImGuiFileDialog)
set(IMGUI_FILE_DIALOG_SOURCES
    ${IMGUI_FILE_DIALOG_DIR}/ImGuiFileDialog.cpp
)
set(IMGUI_FILE_DIALOG_HEADERS
    ${IMGUI_FILE_DIALOG_DIR}/ImGuiFileDialog.h
    ${IMGUI_FILE_DIALOG_DIR}/ImGuiFileDialogConfig.h
)

# Create the UI library
add_library(usd_vk_ui ${UI_SOURCES} ${UI_HEADERS}
    ${IMGUI_SOURCES} ${IMGUI_BACKEND_SOURCES} ${IMGUI_FILE_DIALOG_SOURCES}
)

target_include_directories(usd_vk_ui PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${IMGUI_BACKEND_DIR}
    ${IMGUI_FILE_DIALOG_DIR}
)

target_link_libraries(usd_vk_ui PUBLIC
    usd_vk_core
    usd_vk_render
    Vulkan::Vulkan
    glfw
)

# Disable warnings for ImGui sources
target_compile_options(usd_vk_ui PRIVATE
    -Wno-unused-parameter
    -Wno-sign-compare
    -Wno-missing-field-initializers
)
