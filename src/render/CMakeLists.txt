# Render library - Vulkan

set(RENDER_SOURCES
    vulkan_context.cpp
    vulkan_swapchain.cpp
    vulkan_buffer.cpp
    vulkan_pipeline.cpp
    vulkan_descriptor.cpp
    shader.cpp
    shader_compiler.cpp
    renderer.cpp
)

set(RENDER_HEADERS
    vulkan_context.hpp
    vulkan_swapchain.hpp
    vulkan_buffer.hpp
    vulkan_pipeline.hpp
    vulkan_descriptor.hpp
    shader.hpp
    shader_compiler.hpp
    renderer.hpp
)

# Shader compilation
find_program(GLSLANG_VALIDATOR glslangValidator
    PATHS
        "/usr/bin"
        "/usr/local/bin"
        "/opt/VulkanSDK"
        "$ENV{VULKAN_SDK}/Bin"
        "/usr/lib/x86_64-linux-gnu/glslang"
        "/usr/bin/glslang"
)

if(GLSLANG_VALIDATOR)
    message(STATUS "Found glslangValidator: ${GLSLANG_VALIDATOR}")

    # Find all GLSL shader files (exclude .spv files)
    file(GLOB GLSL_SHADERS
        "${CMAKE_SOURCE_DIR}/src/shaders/*.vert"
        "${CMAKE_SOURCE_DIR}/src/shaders/*.frag"
        "${CMAKE_SOURCE_DIR}/src/shaders/*.geom"
        "${CMAKE_SOURCE_DIR}/src/shaders/*.comp"
        "${CMAKE_SOURCE_DIR}/src/shaders/*.tesc"
        "${CMAKE_SOURCE_DIR}/src/shaders/*.tese"
    )
    # Filter out .spv files if any were incorrectly matched
    list(FILTER GLSL_SHADERS EXCLUDE REGEX "\\.spv$")

    # Create output directory
    set(SPIRV_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")
    file(MAKE_DIRECTORY ${SPIRV_OUTPUT_DIR})

    foreach(GLSL ${GLSL_SHADERS})
        get_filename_component(SHADER_NAME ${GLSL} NAME_WE)
        get_filename_component(SHADER_EXT ${GLSL} EXT)
        set(SPIRV "${SPIRV_OUTPUT_DIR}/${SHADER_NAME}${SHADER_EXT}.spv")

        add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSLANG_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL}
            COMMENT "Compiling shader: ${SHADER_NAME}${SHADER_EXT}"
        )

        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach()

    # Create a custom target for shader compilation
    add_custom_target(shaders DEPENDS ${SPIRV_BINARY_FILES})
else()
    message(WARNING "glslangValidator not found - shaders must be pre-compiled manually")
endif()

add_library(usd_vk_render ${RENDER_SOURCES})
target_include_directories(usd_vk_render PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_vk_render PUBLIC usd_vk_core Vulkan::Vulkan glm::glm)
