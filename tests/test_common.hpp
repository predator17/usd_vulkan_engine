#pragma once

// Test configuration
// TEST_SHADER_DIR is defined by CMake as the build output shader directory
#ifndef TEST_SHADER_DIR
#define TEST_SHADER_DIR "shaders"
#endif

#define SHADER_PATH(name) TEST_SHADER_DIR "/" name
