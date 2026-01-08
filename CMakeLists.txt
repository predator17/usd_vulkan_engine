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

# Include FetchContent
include(FetchContent)

# Find dependencies
find_package(Threads REQUIRED)

# spdlog - use FetchContent if not found
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
    message(STATUS "spdlog not found, fetching with FetchContent")
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.12.0
    )
    # Disable strict format string checking for compatibility
    set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)
    set(SPDLOG_NO_FORMAT_CHECK ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(spdlog)
endif()

# GLM - use FetchContent if not found
find_package(glm QUIET)
if(NOT glm_FOUND)
    message(STATUS "GLM not found, fetching with FetchContent")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
    )
    FetchContent_MakeAvailable(glm)
endif()

# GLFW3
find_package(glfw3 REQUIRED)

# USD
set(USD_ROOT "${CMAKE_SOURCE_DIR}/external/USD/install" CACHE PATH "USD installation path")
if(EXISTS "${USD_ROOT}")
    set(USD_LIB_DIR "${USD_ROOT}/lib")
    set(USD_INCLUDE_DIR "${USD_ROOT}/include")
    link_directories(${USD_LIB_DIR})
    include_directories(${USD_INCLUDE_DIR})
else()
    message(WARNING "USD not found at ${USD_ROOT}")
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
