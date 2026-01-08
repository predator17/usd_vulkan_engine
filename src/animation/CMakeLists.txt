# Animation library

add_library(usd_ve_animation INTERFACE)
target_include_directories(usd_ve_animation INTERFACE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_ve_animation INTERFACE usd_ve_core)
