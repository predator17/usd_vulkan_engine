#==============================================================================
# Test CMakeLists.txt for USD Vulkan Engine
#==============================================================================
# Following TDD methodology with Catch2 v3

# Shader test directory (relative to build directory)
set(TEST_SHADER_DIR "${CMAKE_BINARY_DIR}/shaders")
message(STATUS "Test shader directory: ${TEST_SHADER_DIR}")

#==============================================================================
# Catch2 Configuration
#==============================================================================
# Catch2 is provided by CPM.cmake in root CMakeLists.txt
# CPM creates the Catch2::Catch2 alias automatically

if(TARGET Catch2::Catch2)
    message(STATUS "Catch2 v3 found via CPM.cmake")
else()
    message(FATAL_ERROR "Catch2 target not found. Check root CMakeLists.txt CPM configuration")
endif()

#==============================================================================
# Collect Test Files
#==============================================================================
# Collect all test files (excluding main.cpp which is provided by Catch2::Catch2WithMain)
file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Exclude main.cpp since Catch2::Catch2WithMain provides main()
list(FILTER TEST_SOURCES EXCLUDE REGEX ".*main.cpp$")

#==============================================================================
# Test Creation Functions
#==============================================================================

# Core tests (no USD, no render)
function(add_usd_vk_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name}
        PRIVATE
        usd_vk_core
        Catch2::Catch2WithMain
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

# USD tests (requires USD libraries)
function(add_usd_vk_usd_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name}
        PRIVATE
        usd_vk_core
        usd_vk_usd
        Catch2::Catch2WithMain
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

# Render tests (requires Vulkan and render libraries)
function(add_usd_vk_render_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name}
        PRIVATE
        usd_vk_core
        usd_vk_render
        Catch2::Catch2WithMain
        Vulkan::Vulkan
        Vulkan::Loader
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

#==============================================================================
# Group Test Files by Category
#==============================================================================
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

#==============================================================================
# Create Test Executables
#==============================================================================

if(CORE_TEST_SOURCES)
    add_usd_vk_test(usd_vk_core_tests ${CORE_TEST_SOURCES})
endif()

if(USD_TEST_SOURCES)
    # USD is always available via cmake/UsdConfig.cmake
    add_usd_vk_usd_test(usd_vk_usd_tests ${USD_TEST_SOURCES})
endif()

if(RENDER_TEST_SOURCES)
    add_usd_vk_render_test(usd_vk_render_tests ${RENDER_TEST_SOURCES})
endif()

#==============================================================================
# Summary
#==============================================================================
message(STATUS "Core tests: ${CORE_TEST_SOURCES}")
message(STATUS "USD tests: ${USD_TEST_SOURCES}")
message(STATUS "Render tests: ${RENDER_TEST_SOURCES}")
