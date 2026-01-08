# Animation library

add_library(usd_vk_animation INTERFACE)
target_include_directories(usd_vk_animation INTERFACE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_vk_animation INTERFACE usd_vk_core)
