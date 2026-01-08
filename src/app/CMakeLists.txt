# Application library and executable

set(APP_SOURCES
    application.cpp
)

set(APP_HEADERS
    application.hpp
)

add_library(usd_vk_app ${APP_SOURCES} ${APP_HEADERS})
target_include_directories(usd_vk_app PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_vk_app
    PUBLIC
    usd_vk_core
    usd_vk_render
    usd_vk_scene
)

# Main executable
add_executable(usd_vulkan_engine main.cpp)
target_link_libraries(usd_vulkan_engine
    PRIVATE
    usd_vk_app
    usd_vk_render
    usd_vk_scene
    usd_vk_core
    usd_vk_usd
    Vulkan::Vulkan
    glm::glm
    spdlog::spdlog
    Threads::Threads
    glfw
)

# Depend on shader compilation
add_dependencies(usd_vulkan_engine shaders)

# Link Python library (USD was built with Python support)
if(EXISTS "/usr/lib64/libpython3.9.so")
    target_link_libraries(usd_vulkan_engine PRIVATE /usr/lib64/libpython3.9.so)
endif()
target_include_directories(usd_vulkan_engine
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
