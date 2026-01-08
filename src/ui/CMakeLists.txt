# UI library

add_library(usd_vk_ui INTERFACE)
target_include_directories(usd_vk_ui INTERFACE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_vk_ui INTERFACE usd_vk_core usd_vk_render)
