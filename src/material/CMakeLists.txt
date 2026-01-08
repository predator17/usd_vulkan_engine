# Material library

add_library(usd_vk_material INTERFACE)
target_include_directories(usd_vk_material INTERFACE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_vk_material INTERFACE usd_vk_core)
