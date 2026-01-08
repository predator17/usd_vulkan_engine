# Material library

add_library(usd_ve_material INTERFACE)
target_include_directories(usd_ve_material INTERFACE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_ve_material INTERFACE usd_ve_core)
