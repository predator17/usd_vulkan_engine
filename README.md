# USD Vulkan Engine

A C++20 real-time USD (Universal Scene Description) viewer built with Vulkan rendering.

## Technology Stack

- **Language**: C++20
- **Rendering**: Vulkan-Hpp
- **Scene Format**: Pixar USD
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

# Configure (dependencies are auto-fetched via FetchContent)
cmake ..

# Build
make -j$(nproc)

# Run core tests
./tests/usd_ve_core_tests
```

## Project Status

| Phase | Status | Tests | Notes |
|-------|--------|-------|-------|
| 0: Test Infrastructure | ✅ | - | Catch2 integrated, main.cpp added |
| 1: Core Data Structures | ✅ | 400 | All tests pass (64 test cases) |
| 2: USD Integration | ✅ | - | Implemented, tests pending USD installation |
| 3: Vulkan Foundation | ✅ | 202 | Context, Swapchain tests pass (57 test cases) |
| 4: Vulkan Pipeline | ✅ | - | Buffer, Descriptor, Pipeline classes (included in Phase 3 tests) |
| 5: Shaders | ❌ | 0 | Not started |
| 6: Main Renderer | ❌ | 0 | Not started |
| 7: Scene Integration | ❌ | 0 | Not started |
| 8: End-to-End | ❌ | 0 | Not started |

**Total: 602 assertions passing in 121 test cases**
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

Note: USD tests require USD to be installed at `external/USD/install`.

## Vulkan Rendering

The `usd_ve_render` library provides Vulkan rendering:

- **VulkanContext**: Instance, device, queue management
- **VulkanSwapchain**: Swapchain, depth buffer, render pass, framebuffers
- **VulkanBuffer**: Base buffer class with VertexBuffer, IndexBuffer, UniformBuffer
- **VulkanDescriptor**: Descriptor set layout and pool management
- **VulkanPipeline**: Graphics pipeline with shader stages

## Namespace

All project code is in the `usd_ve` namespace to avoid conflicts with dependencies.

## Testing

```bash
# Run all tests via CTest
LD_LIBRARY_PATH=/path/to/USD/install/lib:$LD_LIBRARY_PATH ctest --output-on-failure

# Run core tests
./tests/usd_ve_core_tests

# Run Vulkan render tests
./tests/usd_ve_render_tests

# Run with verbose output
./tests/usd_ve_core_tests -s

# Run specific test case
./tests/usd_ve_core_tests "Color: To hex"
```

**Test Coverage:**
- Core: 400 assertions in 64 test cases
- Render: 202 assertions in 57 test cases
- **Total: 602 assertions in 121 test cases**

## Dependencies

All dependencies are automatically fetched via CMake FetchContent:
- spdlog (v1.12.0)
- GLM (0.9.9.8)
- Catch2 (v2.13.10)

External dependencies that must be pre-installed:
- Vulkan SDK
- GLFW3
- Pixar USD (optional, for USD tests)

## License

MIT License
