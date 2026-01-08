# Test CMakeLists.txt for USD Vulkan Engine
# Following TDD methodology with Catch2

# Shader test directory (relative to build directory)
set(TEST_SHADER_DIR "${CMAKE_BINARY_DIR}/shaders")
message(STATUS "Test shader directory: ${TEST_SHADER_DIR}")

# Use local Catch2 from external/ (added by external/CMakeLists.txt)
if(TARGET catch2)
    message(STATUS "Using catch2 target from external/")
    add_library(Catch2::Catch2 ALIAS catch2)
    set(Catch2_FOUND TRUE)
else()
    message(FATAL_ERROR "Catch2 target not found. Check external/CMakeLists.txt")
endif()

message(STATUS "Catch2 found: ${Catch2_FOUND}")

# Collect all test files
file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Main file for Catch2
set(TEST_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

# Create individual test executables for each category
function(add_usd_vk_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name}
        PRIVATE
        usd_vk_core
        Catch2::Catch2
        Vulkan::Vulkan
        glm::glm
        spdlog::spdlog
        Threads::Threads
    )
    target_include_directories(${test_name}
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests
    )
    target_compile_definitions(${test_name} PRIVATE
        TEST_SHADER_DIR="${TEST_SHADER_DIR}"
    )
    # Set working directory to build root for shader path resolution
    add_test(NAME ${test_name} COMMAND ${test_name}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    # Enable folder grouping in IDEs
    set_target_properties(${test_name} PROPERTIES FOLDER "Tests")
endfunction()

function(add_usd_vk_usd_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name}
        PRIVATE
        usd_vk_core
        usd_vk_usd
        Catch2::Catch2
        Threads::Threads
    )
    target_include_directories(${test_name}
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests
    )
    target_compile_definitions(${test_name} PRIVATE
        TEST_SHADER_DIR="${TEST_SHADER_DIR}"
    )
    add_test(NAME ${test_name} COMMAND ${test_name}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    set_target_properties(${test_name} PROPERTIES FOLDER "Tests")
endfunction()

function(add_usd_vk_render_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name}
        PRIVATE
        usd_vk_core
        usd_vk_render
        Catch2::Catch2
        Vulkan::Vulkan
        glm::glm
        spdlog::spdlog
        Threads::Threads
        glfw
    )
    target_include_directories(${test_name}
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests
    )
    target_compile_definitions(${test_name} PRIVATE
        TEST_SHADER_DIR="${TEST_SHADER_DIR}"
    )
    # Set working directory to build root for shader path resolution
    add_test(NAME ${test_name} COMMAND ${test_name}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    set_target_properties(${test_name} PROPERTIES FOLDER "Tests")
endfunction()

# Group test files by category
set(CORE_TEST_SOURCES)
set(USD_TEST_SOURCES)
set(RENDER_TEST_SOURCES)

foreach(test_file ${TEST_SOURCES})
    if(test_file MATCHES "tests/core/")
        list(APPEND CORE_TEST_SOURCES ${test_file})
    elseif(test_file MATCHES "tests/usd/")
        list(APPEND USD_TEST_SOURCES ${test_file})
    elseif(test_file MATCHES "tests/render/")
        list(APPEND RENDER_TEST_SOURCES ${test_file})
    endif()
endforeach()

# Create test executables
if(CORE_TEST_SOURCES)
    add_usd_vk_test(usd_vk_core_tests ${TEST_MAIN} ${CORE_TEST_SOURCES})
endif()

if(USD_TEST_SOURCES AND USD_FOUND)
    add_usd_vk_usd_test(usd_vk_usd_tests ${TEST_MAIN} ${USD_TEST_SOURCES})
elseif(USD_TEST_SOURCES)
    message(STATUS "USD tests skipped (USD not found)")
endif()

if(RENDER_TEST_SOURCES)
    add_usd_vk_render_test(usd_vk_render_tests ${TEST_MAIN} ${RENDER_TEST_SOURCES})
endif()

# Summary
message(STATUS "Core tests: ${CORE_TEST_SOURCES}")
message(STATUS "USD tests: ${USD_TEST_SOURCES}")
message(STATUS "Render tests: ${RENDER_TEST_SOURCES}")
