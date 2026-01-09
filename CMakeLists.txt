cmake_minimum_required(VERSION 3.20)
project(usd_vulkan_engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#==============================================================================
# Build Configuration
#==============================================================================
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(USD_VULKAN_ENGINE_BUILD_TESTS "Build tests" ON)
option(USD_VULKAN_ENGINE_VALIDATION "Enable Vulkan validation layers" ON)

# Add our cmake module path
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

#==============================================================================
# CPM.cmake Configuration
#==============================================================================
set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cpm-cache" CACHE PATH "CPM cache directory")
set(CPM_USE_NAMED_CACHE_DIRECTORIES TRUE)  # Better IDE readability
set(CPM_DOWNLOAD_ALL ${DOWNLOAD_ALL} CACHE BOOL "Download all dependencies from internet" INTERNAL)

# CPM.cmake is already downloaded to cmake/CPM.cmake
if(EXISTS "${CMAKE_SOURCE_DIR}/cmake/CPM.cmake")
    include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)
    message(STATUS "CPM.cmake loaded from cmake/CPM.cmake")
else()
    message(FATAL_ERROR "CPM.cmake not found. Please run: ./scripts/download_dependencies.sh")
endif()

#==============================================================================
# Dependencies via CPM.cmake
#==============================================================================

# Catch2 v3.12.0 - Testing framework
CPMAddPackage(
    NAME catch2
    VERSION 3.12.0
    GITHUB_REPOSITORY catchorg/Catch2
    OPTIONS "BUILD_TESTING OFF"
)
if(catch2_ADDED)
    message(STATUS "Catch2 v${catch2_VERSION} via CPM")
endif()

# GLM 1.0.2 - Header-only math library
# Note: GLM tags don't use 'v' prefix, use GIT_TAG directly
CPMAddPackage(
    NAME glm
    GITHUB_REPOSITORY g-truc/glm
    GIT_TAG 1.0.2
)
if(glm_ADDED)
    message(STATUS "GLM v${glm_VERSION} via CPM")
endif()

#==============================================================================
# Vulkan Setup (headers only, no system Vulkan required)
#==============================================================================

# Vulkan-Headers v1.4.337 - Vulkan C API headers (via CPM)
# Note: DOWNLOAD_ONLY prevents running Vulkan-Hpp's CMake which tries to find system Vulkan
CPMAddPackage(
    NAME vulkan-headers
    GITHUB_REPOSITORY KhronosGroup/Vulkan-Headers
    VERSION 1.4.337
    OPTIONS "BUILD_TESTS OFF"
)

# Vulkan-Hpp v1.4.337 - Vulkan C++ bindings (via CPM)
CPMAddPackage(
    NAME vulkan-hpp
    GITHUB_REPOSITORY KhronosGroup/Vulkan-Hpp
    VERSION 1.4.337
    DOWNLOAD_ONLY YES  # Don't run Vulkan-Hpp's CMake (it tries to find system Vulkan)
)

# Create Vulkan::Vulkan target from downloaded CPM sources
# This is a header-only interface - no linking to system Vulkan libraries required
if(vulkan-headers_ADDED)
    add_library(Vulkan_Headers INTERFACE IMPORTED GLOBAL)
    target_include_directories(Vulkan_Headers INTERFACE
        ${vulkan-headers_SOURCE_DIR}/include
    )

    # Add Vulkan-Hpp to include path if downloaded
    if(vulkan-hpp_ADDED)
        target_include_directories(Vulkan_Headers INTERFACE
            ${vulkan-hpp_SOURCE_DIR}
        )
    endif()

    # Create Vulkan::Vulkan as an alias to our header-only interface
    add_library(Vulkan::Vulkan ALIAS Vulkan_Headers)
    # Create Vulkan::Headers alias only if it doesn't already exist
    if(NOT TARGET Vulkan::Headers)
        add_library(Vulkan::Headers ALIAS Vulkan_Headers)
    endif()

    message(STATUS "Vulkan v${vulkan-headers_VERSION} (headers only) via CPM")
    message(STATUS "  Vulkan-Headers: ${vulkan-headers_SOURCE_DIR}/include")
    if(vulkan-hpp_ADDED)
        message(STATUS "  Vulkan-Hpp: ${vulkan-hpp_SOURCE_DIR}")
    endif()
else()
    message(FATAL_ERROR "Vulkan headers not found via CPM. Please run: ./scripts/download_dependencies.sh")
endif()

#==============================================================================
# Windowing (GLFW)
#==============================================================================

# GLFW v3.4 - Window/input library (via CPM for self-contained builds)
# Note: GLFW tags don't use 'v' prefix, use GIT_TAG directly
CPMAddPackage(
    NAME glfw
    GITHUB_REPOSITORY glfw/glfw
    GIT_TAG 3.4
    OPTIONS "GLFW_BUILD_TESTS OFF" "GLFW_BUILD_DOCS OFF" "GLFW_BUILD_EXAMPLES OFF"
)
if(glfw_ADDED)
    message(STATUS "GLFW v${glfw_VERSION} via CPM")
endif()

#==============================================================================
# UI Libraries (depend on Vulkan + GLFW)
#==============================================================================

# Dear ImGui v1.92.2 - Immediate mode GUI (stable release)
CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    VERSION 1.92.2
)
if(imgui_ADDED)
    # Create static library target for Dear ImGui with backends
    if(NOT TARGET imgui)
        add_library(imgui STATIC
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
        )
        target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
        target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)
        message(STATUS "Dear ImGui v${imgui_VERSION} via CPM")
    endif()
    # Export imgui source directory for ImGuiFileDialog
    set(IMGUI_ROOT_DIR ${imgui_SOURCE_DIR} CACHE PATH "ImGui root directory" FORCE)
endif()

# ImGuiFileDialog v0.6.7 - File dialog for ImGui
CPMAddPackage(
    NAME ImGuiFileDialog
    GITHUB_REPOSITORY aiekick/ImGuiFileDialog
    VERSION 0.6.7
)
if(ImGuiFileDialog_ADDED)
    message(STATUS "ImGuiFileDialog v${ImGuiFileDialog_VERSION} via CPM")
    # ImGuiFileDialog needs to find imgui.h - add include directory
    if(TARGET ImGuiFileDialog)
        if(imgui_ADDED)
            target_include_directories(ImGuiFileDialog PRIVATE ${imgui_SOURCE_DIR})
            message(STATUS "  Added imgui include dir to ImGuiFileDialog")
        endif()
    endif()
endif()

# spdlog v1.16.0 - Logging library
CPMAddPackage(
    NAME spdlog
    GITHUB_REPOSITORY gabime/spdlog
    VERSION 1.16.0
    OPTIONS "SPDLOG_BUILD_SHARED OFF" "SPDLOG_BUILD_TESTS OFF" "SPDLOG_INSTALL OFF"
)
if(spdlog_ADDED)
    message(STATUS "spdlog v${spdlog_VERSION} via CPM")
endif()

# Threads (always required)
find_package(Threads REQUIRED)

#==============================================================================
# USD - Local Build (Not managed by CPM)
#==============================================================================
include(${CMAKE_SOURCE_DIR}/cmake/UsdConfig.cmake)

#==============================================================================
# Project Subdirectories
#==============================================================================
add_subdirectory(src)

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
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${CMAKE_SOURCE_DIR}/build/CMakeFiles
        ${CMAKE_SOURCE_DIR}/build/src/CMakeFiles
        ${CMAKE_SOURCE_DIR}/build/tests/CMakeFiles
        ${CMAKE_SOURCE_DIR}/build/external/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${CMAKE_SOURCE_DIR}/src/CMakeFiles
        ${CMAKE_SOURCE_DIR}/src/*/CMakeFiles
        ${CMAKE_SOURCE_DIR}/tests/CMakeFiles
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
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/.cmake
    COMMENT "Tidying up project..."
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# help-cleanup target - show available targets
add_custom_target(help-cleanup
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Cleanup Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  make project-clean - Remove build artifacts, keep build dir"
    COMMAND ${CMAKE_COMMAND} -E echo "  make tidy         - Remove intermediate files, keep binaries"
    COMMAND ${CMAKE_COMMAND} -E echo "  make distclean    - Remove entire build directory"
    COMMAND ${CMAKE_COMMAND} -E echo "  make help-cleanup - Show this help message"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMENT "Cleanup help"
)

# Post-build cleanup - run automatically after build
add_custom_target(postbuild_cleanup
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${CMAKE_SOURCE_DIR}/src/CMakeFiles
        ${CMAKE_SOURCE_DIR}/src/*/CMakeFiles
        ${CMAKE_SOURCE_DIR}/tests/CMakeFiles
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
