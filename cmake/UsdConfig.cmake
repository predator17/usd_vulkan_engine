#==============================================================================
# USD Configuration for USD Vulkan Engine
#==============================================================================
#
# This module configures OpenUSD (Universal Scene Description) for the project.
#
# USD Directory Structure:
#   OpenUSD/
#   ├── src/       # Source tarballs (e.g., OpenUSD-25.11.tar.gz)
#   ├── build/     # Extracted and built sources (e.g., 25.11/)
#   └── install/   # Installed binaries (e.g., 25.11/)
#
# Usage:
#   include(${CMAKE_SOURCE_DIR}/cmake/UsdConfig.cmake)
#
# Provides:
#   - PXR_INCLUDE_DIRS - USD include directories
#   - PXR_LIBRARIES     - List of all USD library targets
#   - pxr::usd, pxr::usdGeom, etc. - Individual USD library targets
#
#==============================================================================

#==============================================================================
# USD Root Configuration
#==============================================================================

# Get USD version from CPM or default to 25.11
if(DEFINED OpenUSD_VERSION)
    set(USD_VERSION "${OpenUSD_VERSION}")
else()
    set(USD_VERSION "25.11")
endif()

# Set USD root directory
if(DEFINED ENV{USD_ROOT})
    # Allow override via environment variable
    set(USD_ROOT "$ENV{USD_ROOT}")
    message(STATUS "USD_ROOT from environment: ${USD_ROOT}")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/OpenUSD/install/${USD_VERSION}")
    # Standard project location
    set(USD_ROOT "${CMAKE_SOURCE_DIR}/OpenUSD/install/${USD_VERSION}")
    message(STATUS "USD_ROOT from project: ${USD_ROOT}")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/external/USD/install")
    # Legacy location (for backward compatibility during migration)
    set(USD_ROOT "${CMAKE_SOURCE_DIR}/external/USD/install")
    message(WARNING "Using legacy USD location: ${USD_ROOT}")
else()
    message(FATAL_ERROR "USD not found at OpenUSD/install/${USD_VERSION} or external/USD/install. "
                        "Please build USD first or set USD_ROOT environment variable.")
endif()

#==============================================================================
# Include Native USD Configuration
#==============================================================================

# USD provides its own CMake config file (pxrConfig.cmake)
# pxrConfig.cmake is located in the USD root install directory

if(EXISTS "${USD_ROOT}/pxrConfig.cmake")
    # Include USD's native configuration which provides all pxr:: targets
    include("${USD_ROOT}/pxrConfig.cmake")

    message(STATUS "USD v${PXR_VERSION} found at ${USD_ROOT}")
    message(STATUS "  Include dirs: ${PXR_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "USD cmake config not found at ${USD_ROOT}/pxrConfig.cmake. "
                        "Please ensure USD is built and installed correctly.")
endif()

#==============================================================================
# USD Library Loading Configuration
#==============================================================================

# For proper runtime loading of USD shared libraries, we need to set RPATH
# This is especially important on Linux where USD libraries have dependencies

if(NOT WIN32)
    # Get USD library directory
    set(USD_LIB_DIR "${USD_ROOT}/lib")

    # Add USD lib directory to build RPATH for proper runtime loading
    # This ensures that USD plugins and libraries can be found at runtime
    set(CMAKE_BUILD_RPATH "${CMAKE_BUILD_RPATH}:${USD_LIB_DIR}")

    # For installation, we may want to copy USD libraries or use a different RPATH
    # This is handled by setting INSTALL_RPATH later if needed

    message(STATUS "USD RPATH configured: ${USD_LIB_DIR}")
endif()

#==============================================================================
# Optional: Create Convenience Target for USD Vulkan Engine
#==============================================================================

# Create an alias target for easier linking
# This allows using USD::Usd instead of the individual pxr:: targets
if(TARGET usd)
    if(NOT TARGET USD::Usd)
        add_library(USD::Usd ALIAS usd)
        message(STATUS "Created USD::Usd alias target")
    endif()
endif()

#==============================================================================
# USD Plugin Path Configuration
#==============================================================================

# Set USD plugin path so USD can find its plugins at runtime
if(NOT DEFINED ENV{PXR_PLUGINPATH})
    # Set plugin path to USD's usd directory
    set(USD_PLUGIN_DIR "${USD_ROOT}/lib/usd")

    if(EXISTS "${USD_PLUGIN_DIR}")
        set(ENV{PXR_PLUGINPATH} "${USD_PLUGIN_DIR}")
        message(STATUS "USD Plugin Path: ${USD_PLUGIN_DIR}")
    endif()
endif()

#==============================================================================
# Summary
#==============================================================================

message(STATUS "USD Configuration Summary:")
message(STATUS "  USD Version: ${PXR_VERSION}")
message(STATUS "  USD Root: ${USD_ROOT}")
message(STATUS "  USD Libraries: ${PXR_LIBRARIES}")
