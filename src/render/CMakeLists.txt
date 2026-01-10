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

#==============================================================================
# Shader Configuration - Supports pre-compiled SPIR-V files
#==============================================================================

# Output directory for compiled SPIR-V shaders
set(SPIRV_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")
file(MAKE_DIRECTORY ${SPIRV_OUTPUT_DIR})

# Shader source directory
set(SHADER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/shaders")

# Find all GLSL shader files
file(GLOB GLSL_SHADERS
    "${SHADER_SOURCE_DIR}/*.vert"
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.geom"
    "${SHADER_SOURCE_DIR}/*.comp"
    "${SHADER_SOURCE_DIR}/*.tesc"
    "${SHADER_SOURCE_DIR}/*.tese"
)

# Process each shader
foreach(GLSL ${GLSL_SHADERS})
    get_filename_component(SHADER_NAME ${GLSL} NAME_WE)
    get_filename_component(SHADER_EXT ${GLSL} EXT)
    set(SPIRV_NAME "${SHADER_NAME}${SHADER_EXT}.spv")

    # Check if pre-compiled SPIR-V exists in source directory
    set(PRECOMPILED_SPIRV "${SHADER_SOURCE_DIR}/${SPIRV_NAME}")
    set(OUTPUT_SPIRV "${SPIRV_OUTPUT_DIR}/${SPIRV_NAME}")

    if(EXISTS "${PRECOMPILED_SPIRV}")
        # Use pre-compiled SPIR-V - copy to build directory
        add_custom_command(
            OUTPUT ${OUTPUT_SPIRV}
            COMMAND ${CMAKE_COMMAND} -E copy ${PRECOMPILED_SPIRV} ${OUTPUT_SPIRV}
            DEPENDS ${PRECOMPILED_SPIRV}
            COMMENT "Copying pre-compiled shader: ${SPIRV_NAME}"
        )
        list(APPEND SPIRV_BINARY_FILES ${OUTPUT_SPIRV})
    else()
        # No pre-compiled SPIR-V - need glslangValidator
        if(NOT GLSLANG_VALIDATOR)
            # Find glslangValidator only once if needed
            find_program(GLSLANG_VALIDATOR glslangValidator
                PATHS
                    "${CMAKE_SOURCE_DIR}/tools/bin"  # Project-local tools
                    "/usr/bin"
                    "/usr/local/bin"
                    "$ENV{VULKAN_SDK}/x86_64/bin"
                    "/opt/VulkanSDK/x86_64/bin"
            )
        endif()

        if(GLSLANG_VALIDATOR)
            message(STATUS "Compiling shader: ${SPIRV_NAME}")
            add_custom_command(
                OUTPUT ${OUTPUT_SPIRV}
                COMMAND ${GLSLANG_VALIDATOR} -V ${GLSL} -o ${OUTPUT_SPIRV}
                DEPENDS ${GLSL}
                COMMENT "Compiling shader: ${SPIRV_NAME}"
            )
            list(APPEND SPIRV_BINARY_FILES ${OUTPUT_SPIRV})
        else()
            message(WARNING "===============================================================================")
            message(WARNING "No pre-compiled SPIR-V found: ${SPIRV_NAME}")
            message(WARNING "glslangValidator not found - cannot compile ${SHADER_NAME}${SHADER_EXT}")
            message(WARNING "Shader will not be available at runtime!")
            message(WARNING "To fix: Either compile GLSL manually or install glslang")
            message(WARNING "===============================================================================")
        endif()
    endif()
endforeach()

# Create custom target for shader processing
if(SPIRV_BINARY_FILES)
    add_custom_target(shaders DEPENDS ${SPIRV_BINARY_FILES})
endif()

add_library(usd_vk_render ${RENDER_SOURCES})
target_include_directories(usd_vk_render PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_vk_render PUBLIC usd_vk_core Vulkan::Vulkan glm::glm)
# Prevent GLFW from including OpenGL headers (Vulkan-only build)
target_compile_definitions(usd_vk_render PUBLIC GLFW_INCLUDE_NONE)
