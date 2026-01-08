# USD Vulkan Engine

A C++20 real-time USD (Universal Scene Description) viewer built with Vulkan rendering.

## Technology Stack

- **Language**: C++20
- **Rendering**: Vulkan-Hpp
- **Scene Format**: Pixar USD v25.11 (built without Python support)
- **Math**: GLM 0.9.9.8
- **Logging**: spdlog v1.12.0
- **Window**: GLFW3
- **UI**: Dear ImGui v1.91.x, ImGuiFileDialog
- **Testing**: Catch2 v2.13.10
- **Build**: CMake 3.20+

## Features

### Implemented
- ✅ Real-time Vulkan rendering with complete graphics pipeline
- ✅ **Orbit camera** with mouse controls (rotate, pan, zoom)
- ✅ **In-engine UI** with ImGui (file browser, playback controls)
- ✅ **Wireframe display mode** toggle
- ✅ **Blue metal shader** with PBR-style lighting
- ✅ Animation playback controls (play/pause, frame stepping)
- ✅ Shader hot-reload support during development
- ✅ Spinning cube demo
- ✅ Comprehensive test suite (677 assertions across 137 tests)

### Partially Implemented
- ⚠️ USD scene file loading (API ready, parser uses hardcoded demo)

### Quick Start

```bash
# Clone or navigate to project
cd usd_vulkan_engine

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
make -j$(nproc)

# Run the engine
LD_LIBRARY_PATH=../external/USD/install/lib:$LD_LIBRARY_PATH ./src/app/usd_vulkan_engine
```

## Controls

### Keyboard Controls

| Key | Action |
|-----|--------|
| **ESC** | Exit application |
| **SPACE** | Pause/Play animation |
| **LEFT** | Step to previous frame |
| **RIGHT** | Step to next frame |

### Mouse Controls

| Input | Action |
|-------|--------|
| **Left Click + Drag** | Orbit camera (rotate around target) |
| **Right Click + Drag** | Pan camera |
| **Scroll Wheel** | Zoom in/out |

### UI Controls

| Menu | Action |
|------|--------|
| **File → Open USD...** | Open file dialog to load USD files |
| **View → Wireframe Mode** | Toggle between solid and wireframe rendering |
| **View → Show Stats** | Toggle stats display |

## Cleanup

The project includes automatic post-build cleanup to keep source directories clean.

### Shell Script (Recommended)

```bash
./cleanup.sh tidy       # Light cleanup: remove CMakeFiles from source, logs, temps
./cleanup.sh sweep      # Aggressive cleanup: all artifacts and intermediates
./cleanup.sh distclean  # Remove everything including build directory
./cleanup.sh help       # Show all options
```

### CMake Targets

```bash
make project-clean   # Remove build artifacts, keep build dir
make tidy            # Remove intermediate files, keep binaries
make sweep           # Aggressive cleanup of all artifacts
make distclean       # Remove entire build directory
make help-cleanup    # Show cleanup help
```

## Dependencies Setup

### System Dependencies

```bash
# Vulkan SDK (headers + glslangValidator)
# Ubuntu/Debian:
sudo apt install vulkan-sdk glslang-tools

# Fedora:
sudo dnf install vulkan-devel glslang

# Arch:
sudo pacman -S vulkan-headers glslang

# GLFW3
# Ubuntu/Debian:
sudo apt install libglfw3-dev

# Fedora:
sudo dnf install glfw-devel

# Arch:
sudo pacman -S glfw-x11
```

### External Dependencies (external/)

```bash
# In project root
mkdir -p external
cd external

# spdlog (logging)
wget https://github.com/gabime/spdlog/archive/refs/tags/v1.12.0.tar.gz -O spdlog.tar.gz
tar xf spdlog.tar.gz && mv spdlog-1.12.0 spdlog

# GLM (math library)
git clone --depth 1 --branch v0.9.9.8 https://github.com/g-truc/glm.git

# Catch2 (testing)
git clone --depth 1 --branch v2.x https://github.com/catchorg/Catch2.git

# ImGui (UI)
git clone --depth 1 https://github.com/ocornut/imgui.git

# ImGuiFileDialog (file dialog)
git clone --depth 1 https://github.com/aiekick/ImGuiFileDialog.git
```

### USD Installation (Required)

USD v25.11 is required and should be installed at `external/USD/install`:

```bash
# In external/
git clone --depth 1 --branch v25.11.0 https://github.com/PixarAnimationStudios/OpenUSD.git USD
cd USD

# Build without Python support to avoid linking issues
mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DPXR_BUILD_USD_USDA=ON \
  -DPXR_BUILD_USD_USDC=ON \
  -DPXR_BUILD_USD_USDZ=ON \
  -DPXR_ENABLE_PYTHON_SUPPORT=OFF \
  -DPXR_BUILD_IMAGING=OFF \
  -DPXR_BUILD_USDVIEW=OFF \
  -DPXR_BUILD_USDRENDERER=OFF \
  -DPXR_BUILD_TESTS=OFF \
  -DPXR_BUILD_EXAMPLES=OFF \
  -DCMAKE_INSTALL_PREFIX=$PWD/../install

make -j$(nproc)
make install
```

## Project Status

| Phase | Status | Tests | Notes |
|-------|--------|-------|-------|
| 0: Test Infrastructure | ✅ | - | Catch2 integrated |
| 1: Core Data Structures | ✅ | 400 | Color, Bounds, Transform, Timer, Logger |
| 2: USD Integration | ⚠️ | - | API ready, parser needs implementation |
| 3: Vulkan Foundation | ✅ | 202 | Context, Swapchain, Framebuffer |
| 4: Vulkan Pipeline | ✅ | 95 | Buffer, Descriptor, Pipeline |
| 5: Shaders | ✅ | - | Blue metal shader with PBR lighting |
| 6: Main Renderer | ✅ | - | Wireframe mode, dual pipeline system |
| 7: Camera System | ✅ | - | Orbit controls with mouse input |
| 8: UI System | ✅ | - | ImGui integration, file browser |
| 9: Scene Integration | ✅ | - | Scene loading, mesh rendering |
| 10: End-to-End | ✅ | - | Working viewer with all controls |

**Test Results:**
- Core: 400 assertions in 64 test cases
- Render: 277 assertions in 73 test cases
- **Total: 677 assertions in 137 test cases**

**Progress: ~90% Complete** (Full USD parser pending)

## Known Limitations & TODO

### TODO Items

1. **USD Parser Implementation** (`src/scene/scene.cpp`)
   - Current: Hardcoded test cube for demonstration
   - Needed: Proper USDA/USDC file parsing with mesh extraction
   - Priority: HIGH for USD file support

2. **USD Stage Loading** (`src/usd/stage.cpp`)
   - UsdStageWrapper::load() needs implementation
   - Should leverage USD C++ API for proper scene loading

3. **Animation System** (`src/animation/player.hpp`)
   - Placeholder only
   - Needed: Time sampled attribute playback from USD

4. **Material System** (`src/material/material.hpp`)
   - Placeholder only
   - Needed: PBR material support from USD

### Current Workarounds

- Scene creates a hardcoded spinning cube instead of parsing USD files
- Animation is procedural (rotation based on time) rather than USD-driven
- Single material (blue metal) for all geometry

## Architecture

### Core Library (`usd_vk_core`)

Foundational data structures:

- **Color**: RGBA with sRGB/linear conversion, hex encoding, luminance
- **Bounds**: AABB with expansion and intersection tests
- **Transform**: TRS hierarchy with parent-child relationships
- **Timer**: High-resolution timer with scoped timing
- **Logger**: spdlog wrapper for consistent logging
- **Assert**: Custom assertion system with debug breakpoint support

### Camera System (`usd_vk_camera`)

- **Camera**: Perspective camera with orbit controls
  - Spherical coordinates (radius, azimuth, elevation)
  - Mouse input handling for orbit, pan, and zoom
  - View and projection matrix generation

### UI System (`usd_vk_ui`)

Complete Dear ImGui integration:

- **ImGuiContext**: ImGui context and GLFW backend management
- **ImGuiRenderer**: Vulkan-specific rendering for ImGui
- **ImGuiUI**: Main UI implementation
  - Menu bar with File, View, Help menus
  - File browser dialog using ImGuiFileDialog
  - Playback controls (play/pause, frame slider, step buttons)
  - Stats display (FPS, frame info, display mode)
- **ImGuiFileDialog**: Wrapper for file selection dialog

### USD Integration (`usd_vk_usd`)

USD scene loading interface (pimpl pattern):

- **Stage**: USD stage loading interface
- **Mesh**: Mesh extraction API

Note: Current implementation uses placeholder with hardcoded demo data.

### Vulkan Rendering (`usd_vk_render`)

Complete Vulkan rendering system:

- **VulkanContext**: Instance, device, queue, debug messenger
- **VulkanSwapchain**: Swapchain, depth buffer, render pass, framebuffers
- **VulkanBuffer**: Base buffer with VertexBuffer, IndexBuffer, UniformBuffer
- **VulkanDescriptor**: Descriptor set layout and pool management
- **VulkanPipeline**: Graphics pipeline with configurable polygon mode (fill/wireframe)
- **Shader**: SPIR-V shader loading with hot-reload support
- **Renderer**: Main rendering orchestrator
  - Dual pipeline system (solid fill + wireframe)
  - Push constants for transform matrices
  - Command buffer recording and synchronization

### Scene Management (`usd_vk_scene`)

- **Scene**: Scene container with MeshData array
- **MeshData**: Vertices, normals, colors, texcoords, indices, transforms
- **Animation**: Time-based animation updates

### Application (`usd_vk_app`)

- **Application**: Main application class
  - Window creation and event handling (GLFW)
  - Main loop with delta time
  - Keyboard input (ESC, SPACE, LEFT, RIGHT)
  - Mouse input (orbit camera controls)
  - ImGui integration
  - Animation controls (play/pause, frame step)
  - Graceful shutdown on signals

## Shaders

Located in `src/shaders/`:

### basic.vert
Vertex shader with full transform support:
- Inputs: position, color, normal
- Push constants: model, view, projection matrices, time
- Outputs: color, normal, world position to fragment shader
- Normal transformation to world space

### basic.frag
Blue metal fragment shader with PBR-style lighting:
- **Three-point lighting**: Main, fill, and accent lights
- **Blinn-Phong specular**: High shininess for metallic appearance
- **Fresnel-Schlick**: Metallic edge effects
- **Rim lighting**: Blue-tinted edge glow
- **Tone mapping**: Reinhard tone mapping
- **Gamma correction**: sRGB output

Constants:
- Base color: Muted blue (0.2, 0.35, 0.6)
- Metal color: Bright blue (0.4, 0.6, 0.9)
- Metalness: 0.9
- Roughness: 0.35
- Specular strength: 2.2

Shaders are compiled to SPIR-V at build time to `build/shaders/`.

## Testing

```bash
# Run all tests via CTest
LD_LIBRARY_PATH=../external/USD/install/lib:$LD_LIBRARY_PATH ctest --output-on-failure

# Run specific test suite
./tests/usd_vk_core_tests -s
./tests/usd_vk_render_tests -s

# Run specific test case
./tests/usd_vk_render_tests "Shader: Load from SPIR-V"

# Run shader tests only
./tests/usd_vk_render_tests "[shader]"
```

## Build Configuration Options

```bash
# Enable validation layers (default: ON)
cmake -DUSD_VULKAN_ENGINE_VALIDATION=ON ..

# Disable tests (default: ON)
cmake -DUSD_VULKAN_ENGINE_BUILD_TESTS=OFF ..
```

## Project Structure

```
usd_vulkan_engine/
├── src/
│   ├── app/           # Application entry point, main loop
│   ├── core/          # Core data structures
│   ├── usd/           # USD integration layer
│   ├── render/        # Vulkan rendering
│   ├── scene/         # Scene management
│   ├── camera/        # Camera system
│   ├── shaders/       # GLSL source
│   ├── ui/            # ImGui UI integration
│   ├── animation/     # Animation (placeholder)
│   └── material/      # Material (placeholder)
├── tests/             # Test suite
├── external/          # Third-party dependencies
├── cmake/             # CMake modules
├── data/              # Sample USD files
└── build/             # Build artifacts
```

## Namespace

All project code uses the `usd_vk` namespace to avoid conflicts.

## License

MIT License
