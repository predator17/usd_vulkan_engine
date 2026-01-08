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
