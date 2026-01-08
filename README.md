# USD Vulkan Engine

A C++20 real-time USD (Universal Scene Description) viewer built with Vulkan rendering.

## Technology Stack

- **Language**: C++20
- **Rendering**: Vulkan-Hpp
- **Scene Format**: Pixar USD (optional)
- **Math**: GLM 0.9.9.8
- **Logging**: spdlog v1.12.0
- **Testing**: Catch2 v2.13.10
- **Build**: CMake 3.20+

## Build

```bash
# Clone or navigate to project
cd usd_vulkan_engine

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build (includes shader compilation)
make -j$(nproc)

# Run tests
LD_LIBRARY_PATH=/path/to/USD/install/lib:$LD_LIBRARY_PATH ctest --output-on-failure

# Or run individual test suites
./tests/usd_ve_core_tests
./tests/usd_ve_render_tests
```

### Dependencies Setup

Dependencies are in `external/` directory. If missing:

```bash
# In project root
mkdir -p external
cd external

# Download dependencies
wget https://github.com/gabime/spdlog/archive/refs/tags/v1.12.0.tar.gz -O spdlog.tar.gz
wget https://github.com/PixarUSD/USD/archive/refs/tags/20.08.tar.gz -O USD.tar.gz
git clone --depth 1 --branch v0.9.9.8 https://github.com/g-truc/glm.git
git clone --depth 1 --branch v2.x https://github.com/catchorg/Catch2.git

# Extract
tar xf spdlog.tar.gz && mv spdlog-1.12.0 spdlog
tar xf USD.tar.gz && mv USD-20.08 USD
```

## Project Status

| Phase | Status | Tests | Notes |
|-------|--------|-------|-------|
| 0: Test Infrastructure | ✅ | - | Catch2 integrated |
| 1: Core Data Structures | ✅ | 400 | Color, Bounds, Transform, Timer, Logger |
| 2: USD Integration | ✅ | - | Stage, Mesh, Material, Transform extraction |
| 3: Vulkan Foundation | ✅ | 202 | Context, Swapchain, Framebuffer |
| 4: Vulkan Pipeline | ✅ | - | Buffer, Descriptor, Pipeline |
| 5: Shaders | ✅ | 95 | Shader class, compiler, hot-reload, VulkanPipeline integration |
| 6: Main Renderer | ❌ | 0 | Not started |
| 7: Scene Integration | ❌ | 0 | Not started |
| 8: End-to-End | ❌ | 0 | Not started |

**Test Results:**
- Core: 400 assertions in 64 test cases
- Render: 297 assertions in 73 test cases
- **Total: 697 assertions in 137 test cases**

**Progress: ~50% Complete**

## Core Library

The `usd_ve_core` library provides foundational data structures:

- **Color**: RGBA color with sRGB/linear conversion, hex encoding, luminance
- **Bounds**: Axis-aligned bounding box with expansion and intersection tests
- **Transform**: TRS (translation, rotation, scale) with parent-child hierarchy
- **Timer**: High-resolution timer with scoped timing utility
- **Logger**: spdlog wrapper for consistent logging

## USD Integration

The `usd_ve_usd` library provides USD scene loading:

- **Stage**: USD stage loading and traversal
- **Mesh Extraction**: Extract mesh data from USD prims
- **Material Extraction**: Extract material data from USD prims
- **Transform Extraction**: Extract transform hierarchies

Note: USD features are optional and require USD to be installed at `external/USD/install`.

## Vulkan Rendering

The `usd_ve_render` library provides Vulkan rendering:

- **VulkanContext**: Instance, device, queue management
- **VulkanSwapchain**: Swapchain, depth buffer, render pass, framebuffers
- **VulkanBuffer**: Base buffer class with VertexBuffer, IndexBuffer, UniformBuffer
- **VulkanDescriptor**: Descriptor set layout and pool management
- **VulkanPipeline**: Graphics pipeline with shader attachment support
- **Shader**: SPIR-V/GLSL shader loading with hot-reload support
- **ShaderCompiler**: GLSL to SPIR-V compilation utilities

### Shaders

Shaders are compiled at build time using glslangValidator:

- Location: `src/shaders/*.vert`, `*.frag`, etc.
- Output: `build/shaders/*.vert.spv`, `*.frag.spv`
- Hot-reload: Call `Shader::checkForReload()` or `VulkanPipeline::checkForShaderReload()`

## Namespace

All project code is in the `usd_ve` namespace to avoid conflicts with dependencies.

## Testing

```bash
# Run all tests via CTest
LD_LIBRARY_PATH=/path/to/USD/install/lib:$LD_LIBRARY_PATH ctest --output-on-failure

# Run with verbose output
./tests/usd_ve_render_tests -s

# Run specific test case
./tests/usd_ve_render_tests "Shader: Load from SPIR-V"

# Run shader tests only
./tests/usd_ve_render_tests "[shader]"
```

## Dependencies

### Local (external/)
- spdlog (v1.12.0) - Logging
- GLM (0.9.9.8) - Math library
- Catch2 (v2.13.10) - Testing framework

### System Required
- Vulkan SDK - For glslangValidator and Vulkan headers
- GLFW3 - Window management

### Optional
- Pixar USD - For USD scene loading (set `USD_ROOT` CMake variable)

## License

MIT License
