#==============================================================================
# USD Library - Universal Scene Description Integration
#==============================================================================

set(USD_SOURCES
    stage.cpp
    mesh.cpp
)

set(USD_HEADERS
    stage.hpp
    mesh.hpp
)

add_library(usd_vk_usd ${USD_SOURCES})
target_include_directories(usd_vk_usd PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_vk_usd PUBLIC usd_vk_core)

#==============================================================================
# USD Libraries - Modern Target-Based Linking
#==============================================================================
# USD targets are provided by pxrConfig.cmake (included in cmake/UsdConfig.cmake)
# Available targets: usd, usdGeom, sdf, pcp, tf, gf, vt, arch, js, kind, etc.

if(TARGET usd)
    # Use modern target-based linking with USD's CMake targets
    # The 'usd' target automatically links its dependencies (transitive)
    target_link_libraries(usd_vk_usd PUBLIC usd)

    # Additional USD libraries that may be needed (linking via 'usd' covers most)
    # If specific USD modules are needed, they can be added here:
    # target_link_libraries(usd_vk_usd PUBLIC usdGeom sdf pcp tf gf vt)

    message(STATUS "USD libraries linked via modern target 'usd'")
else()
    message(FATAL_ERROR "USD target 'usd' not found. "
                        "Ensure USD is built and pxrConfig.cmake is properly included.")
endif()
