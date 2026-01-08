cmake_minimum_required(VERSION 3.20)
project(usd_vulkan_engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(USD_VULKAN_ENGINE_BUILD_TESTS "Build tests" ON)
option(USD_VULKAN_ENGINE_VALIDATION "Enable Vulkan validation layers" ON)

# Add our cmake module path
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Include FetchContent
include(FetchContent)

# Find dependencies
find_package(Threads REQUIRED)

# spdlog - use local version from external/
if(EXISTS "${CMAKE_SOURCE_DIR}/external/spdlog/include/spdlog/spdlog.h")
    message(STATUS "Using local spdlog from external/spdlog")
    set(SPDLOG_FOUND TRUE)
    add_library(spdlog INTERFACE)
    target_include_directories(spdlog INTERFACE "${CMAKE_SOURCE_DIR}/external/spdlog/include")
    add_library(spdlog::spdlog ALIAS spdlog)
else()
    message(FATAL_ERROR "spdlog not found in external/spdlog. Please download it.")
endif()

# GLM - use local version from external/
if(EXISTS "${CMAKE_SOURCE_DIR}/external/glm/glm/glm.hpp")
    message(STATUS "Using local GLM from external/glm")
    set(GLM_FOUND TRUE)
    add_library(glm INTERFACE)
    target_include_directories(glm INTERFACE "${CMAKE_SOURCE_DIR}/external/glm")
    add_library(glm::glm ALIAS glm)
else()
    message(FATAL_ERROR "GLM not found in external/glm. Please download it.")
endif()

# GLFW3
find_package(glfw3 REQUIRED)

# USD (CORE DEPENDENCY - required for USD loading support)
set(USD_ROOT "${CMAKE_SOURCE_DIR}/external/USD/install" CACHE PATH "USD installation path")
if(EXISTS "${USD_ROOT}")
    set(USD_LIB_DIR "${USD_ROOT}/lib")
    set(USD_INCLUDE_DIR "${USD_ROOT}/include")
    link_directories(${USD_LIB_DIR})
    include_directories(${USD_INCLUDE_DIR})
    set(USD_FOUND TRUE)
    message(STATUS "USD found at ${USD_ROOT}")
    # Add RPATH for USD libraries
    set(CMAKE_INSTALL_RPATH "${USD_LIB_DIR}")
    set(CMAKE_BUILD_RPATH "${USD_LIB_DIR}")
else()
    set(USD_FOUND FALSE)
    message(FATAL_ERROR "USD not found at ${USD_ROOT}. USD is a core dependency of this project.")
    message(FATAL_ERROR "  Please build USD in external/USD/install before continuing.")
endif()

# Vulkan
find_package(Vulkan REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
)

# Subdirectories
add_subdirectory(external)
add_subdirectory(src)

# Tests
if(USD_VULKAN_ENGINE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

#==============================================================================
# Cleanup Targets
#==============================================================================

# Custom project-clean target - removes build artifacts but keeps build directory
add_custom_target(project-clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove
        ${CMAKE_BINARY_DIR}/CMakeCache.txt
        ${CMAKE_BINARY_DIR}/cmake_install.cmake
        ${CMAKE_BINARY_DIR}/Makefile
        ${CMAKE_BINARY_DIR}/CTestTestfile.cmake
        ${CMAKE_BINARY_DIR}/_deps
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${CMAKE_BINARY_DIR}/src/*/CMakeFiles
        ${CMAKE_BINARY_DIR}/tests/CMakeFiles
        ${CMAKE_BINARY_DIR}/external/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${CMAKE_SOURCE_DIR}/build/CMakeFiles
        ${CMAKE_SOURCE_DIR}/build/src/CMakeFiles
        ${CMAKE_SOURCE_DIR}/build/tests/CMakeFiles
        ${CMAKE_SOURCE_DIR}/build/external/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove
        ${CMAKE_SOURCE_DIR}/build/CMakeCache.txt
        ${CMAKE_SOURCE_DIR}/build/cmake_install.cmake
        ${CMAKE_SOURCE_DIR}/build/Makefile
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${CMAKE_SOURCE_DIR}/src/CMakeFiles
        ${CMAKE_SOURCE_DIR}/src/*/CMakeFiles
        ${CMAKE_SOURCE_DIR}/tests/CMakeFiles
        ${CMAKE_SOURCE_DIR}/external/CMakeFiles
    COMMENT "Cleaning build artifacts..."
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# distclean target - removes entire build directory
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${CMAKE_SOURCE_DIR}/build
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${CMAKE_SOURCE_DIR}/src/CMakeFiles
        ${CMAKE_SOURCE_DIR}/src/*/CMakeFiles
        ${CMAKE_SOURCE_DIR}/tests/CMakeFiles
        ${CMAKE_SOURCE_DIR}/external/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove
        ${CMAKE_SOURCE_DIR}/CMakeCache.txt
        ${CMAKE_SOURCE_DIR}/cmake_install.cmake
        ${CMAKE_SOURCE_DIR}/Makefile
    COMMENT "Performing distclean..."
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# tidy target - removes intermediate files but keeps compiled binaries
add_custom_target(tidy
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${CMAKE_BINARY_DIR}/CMakeFiles
        ${CMAKE_BINARY_DIR}/_deps
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${CMAKE_SOURCE_DIR}/build/CMakeFiles
        ${CMAKE_SOURCE_DIR}/build/src/CMakeFiles
        ${CMAKE_SOURCE_DIR}/build/tests/CMakeFiles
        ${CMAKE_SOURCE_DIR}/build/external/CMakeFiles
    COMMAND find ${CMAKE_SOURCE_DIR}/external -name "*.log" -delete
    COMMAND find ${CMAKE_SOURCE_DIR}/external -name "latest" -delete
    COMMAND find ${CMAKE_SOURCE_DIR} -maxdepth 1 -name "*.log" -delete
    COMMAND find ${CMAKE_SOURCE_DIR} -maxdepth 1 -name "*.zip" -delete
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/.cmake
    COMMENT "Tidying up project..."
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# sweep target - aggressive cleanup of all temp/artifact files
add_custom_target(sweep
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${CMAKE_SOURCE_DIR}/src/CMakeFiles
        ${CMAKE_SOURCE_DIR}/src/*/CMakeFiles
        ${CMAKE_SOURCE_DIR}/tests/CMakeFiles
        ${CMAKE_SOURCE_DIR}/external/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove
        ${CMAKE_SOURCE_DIR}/CMakeCache.txt
        ${CMAKE_SOURCE_DIR}/cmake_install.cmake
        ${CMAKE_SOURCE_DIR}/Makefile
    COMMAND find ${CMAKE_SOURCE_DIR}/src -name "cmake_install.cmake" -delete
    COMMAND find ${CMAKE_SOURCE_DIR}/src -name "Makefile" -delete
    COMMAND find ${CMAKE_SOURCE_DIR}/tests -name "cmake_install.cmake" -delete
    COMMAND find ${CMAKE_SOURCE_DIR}/tests -name "Makefile" -delete
    COMMAND find ${CMAKE_SOURCE_DIR}/external -name "CMakeCache.txt" -delete
    COMMAND find ${CMAKE_SOURCE_DIR}/external -name "cmake_install.cmake" -delete
    COMMAND find ${CMAKE_SOURCE_DIR}/external -name "Makefile" -delete
    COMMAND find ${CMAKE_SOURCE_DIR}/external/build -type d -delete 2>/dev/null || true
    COMMAND find ${CMAKE_SOURCE_DIR}/external -name "*.log" -delete
    COMMAND find ${CMAKE_SOURCE_DIR}/external -name "latest" -delete
    COMMAND find ${CMAKE_SOURCE_DIR} -maxdepth 1 -name "*.log" -delete
    COMMAND find ${CMAKE_SOURCE_DIR} -maxdepth 1 -name "*.zip" -delete
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/.cmake
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/_deps
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/cmake-build-debug
    COMMENT "Sweeping all artifacts..."
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# help-cleanup target - show available targets
add_custom_target(help-cleanup
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Cleanup Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  make project-clean - Remove build artifacts, keep build dir"
    COMMAND ${CMAKE_COMMAND} -E echo "  make tidy         - Remove intermediate files, keep binaries"
    COMMAND ${CMAKE_COMMAND} -E echo "  make sweep        - Aggressive cleanup of all artifacts"
    COMMAND ${CMAKE_COMMAND} -E echo "  make distclean    - Remove entire build directory"
    COMMAND ${CMAKE_COMMAND} -E echo "  make help-cleanup - Show this help message"
    COMMAND ${CMAKE_COMMAND} -E echo "  ./cleanup.sh - Shell script alternative"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMENT "Cleanup help"
)

# Post-build cleanup - run automatically after build
add_custom_target(postbuild_cleanup
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${CMAKE_SOURCE_DIR}/src/CMakeFiles
        ${CMAKE_SOURCE_DIR}/src/*/CMakeFiles
        ${CMAKE_SOURCE_DIR}/tests/CMakeFiles
        ${CMAKE_SOURCE_DIR}/external/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove
        ${CMAKE_SOURCE_DIR}/CMakeCache.txt
        ${CMAKE_SOURCE_DIR}/cmake_install.cmake
        ${CMAKE_SOURCE_DIR}/Makefile
    COMMAND find ${CMAKE_SOURCE_DIR}/src -name "cmake_install.cmake" -delete
    COMMAND find ${CMAKE_SOURCE_DIR}/src -name "Makefile" -delete
    COMMAND find ${CMAKE_SOURCE_DIR}/tests -name "cmake_install.cmake" -delete
    COMMAND find ${CMAKE_SOURCE_DIR}/tests -name "Makefile" -delete
    COMMENT "Post-build cleanup: removing source dir artifacts..."
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Add postbuild_cleanup as dependency to the main executable
add_dependencies(usd_vulkan_engine postbuild_cleanup)
