# External dependencies

# Catch2 - header only (v2.13.10 uses single_include)
add_library(catch2 INTERFACE)
target_include_directories(catch2 INTERFACE ${CMAKE_SOURCE_DIR}/external/catch2/single_include)
