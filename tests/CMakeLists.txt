# Test CMakeLists.txt for USD Vulkan Engine
# Following TDD methodology with Catch2

# Find Catch2 (first try find_package, then add_subdirectory)
find_package(Catch2 2.13.10 QUIET)

if(NOT Catch2_FOUND)
    # Use FetchContent to download Catch2 if not found
    include(FetchContent)
    FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.13.10
    )
    FetchContent_MakeAvailable(catch2)
    set(Catch2_FOUND TRUE)
endif()

message(STATUS "Catch2 found: ${Catch2_FOUND}")

# Collect all test files
file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Main file for Catch2
set(TEST_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

# Create individual test executables for each category
function(add_usd_ve_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name}
        PRIVATE
        usd_ve_core
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
    add_test(NAME ${test_name} COMMAND ${test_name})
    # Enable folder grouping in IDEs
    set_target_properties(${test_name} PROPERTIES FOLDER "Tests")
endfunction()

function(add_usd_ve_usd_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name}
        PRIVATE
        usd_ve_core
        usd_ve_usd
        Catch2::Catch2
        Threads::Threads
    )
    target_include_directories(${test_name}
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_target_properties(${test_name} PROPERTIES FOLDER "Tests")
endfunction()

function(add_usd_ve_render_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name}
        PRIVATE
        usd_ve_core
        usd_ve_render
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
    add_test(NAME ${test_name} COMMAND ${test_name})
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
    add_usd_ve_test(usd_ve_core_tests ${TEST_MAIN} ${CORE_TEST_SOURCES})
endif()

if(USD_TEST_SOURCES AND EXISTS "${USD_LIB_DIR}")
    add_usd_ve_usd_test(usd_ve_usd_tests ${TEST_MAIN} ${USD_TEST_SOURCES})
elseif(USD_TEST_SOURCES)
    message(STATUS "USD tests skipped (USD not found)")
endif()

if(RENDER_TEST_SOURCES)
    add_usd_ve_render_test(usd_ve_render_tests ${TEST_MAIN} ${RENDER_TEST_SOURCES})
endif()

# Summary
message(STATUS "Core tests: ${CORE_TEST_SOURCES}")
message(STATUS "USD tests: ${USD_TEST_SOURCES}")
message(STATUS "Render tests: ${RENDER_TEST_SOURCES}")
