# External dependencies

# Catch2 - header only
add_library(catch2 INTERFACE)
target_include_directories(catch2 INTERFACE ${CMAKE_SOURCE_DIR}/external/catch2)
