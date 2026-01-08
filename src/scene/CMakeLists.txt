# Scene library

add_library(usd_ve_scene INTERFACE)
target_include_directories(usd_ve_scene INTERFACE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_ve_scene INTERFACE usd_ve_core usd_ve_usd)
