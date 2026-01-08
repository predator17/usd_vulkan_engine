# Render library - Vulkan

set(RENDER_SOURCES
    vulkan_context.cpp
    vulkan_swapchain.cpp
    vulkan_buffer.cpp
    vulkan_pipeline.cpp
    vulkan_descriptor.cpp
)

set(RENDER_HEADERS
    vulkan_context.hpp
    vulkan_swapchain.hpp
    vulkan_buffer.hpp
    vulkan_pipeline.hpp
    vulkan_descriptor.hpp
)

add_library(usd_ve_render ${RENDER_SOURCES})
target_include_directories(usd_ve_render PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_ve_render PUBLIC usd_ve_core Vulkan::Vulkan glm::glm)
