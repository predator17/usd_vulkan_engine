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

# ImGui and ImGuiFileDialog are provided by CPM
# These targets are created by the root CMakeLists.txt via CPM
# The sources are compiled into the imgui and ImGuiFileDialog targets

# Create the UI library
add_library(usd_vk_ui ${UI_SOURCES} ${UI_HEADERS})

target_include_directories(usd_vk_ui PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(usd_vk_ui PUBLIC
    usd_vk_core
    usd_vk_render
    Vulkan::Vulkan
    glfw
    imgui
    ImGuiFileDialog
)

# Disable warnings for ImGui sources
target_compile_options(usd_vk_ui PRIVATE
    -Wno-unused-parameter
    -Wno-sign-compare
    -Wno-missing-field-initializers
)
