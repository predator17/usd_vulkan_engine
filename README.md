# USD Vulkan Engine

A C++20 real-time USD (Universal Scene Description) viewer built with Vulkan rendering.

## Technology Stack

- **Language**: C++20
- **Rendering**: Vulkan-Hpp v1.4.337
- **Scene Format**: Pixar USD v25.11
- **Math**: GLM 1.0.2
- **Logging**: spdlog v1.16.0
- **Window**: GLFW 3.4
- **UI**: Dear ImGui v1.92.2, ImGuiFileDialog v0.6.7
- **Testing**: Catch2 v3.12.0
- **Build**: CMake 3.20+ with CPM.cmake package manager

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
- ✅ Comprehensive test suite (400+ assertions)

### Partially Implemented
- ⚠️ USD scene file loading (API ready, parser uses hardcoded demo)

## Quick Start

### Prerequisites

**System Requirements:**
- C++20 compatible compiler (GCC 11+, Clang 13+, MSVC 2022+)
- CMake 3.20+
- Vulkan SDK (for glslangValidator shader compiler)
- Git

**Install Vulkan SDK:**
```bash
# Ubuntu/Debian:
sudo apt install vulkan-sdk glslang-tools

# Fedora:
sudo dnf install vulkan-devel glslang

# Arch:
sudo pacman -S vulkan-headers glslang
```

### Initial Setup (First Time Only)

**Option 1: Automatic dependency download (Recommended)**
```bash
cd usd_vulkan_engine

# Download all dependencies (CPM packages + USD source)
./scripts/download_dependencies.sh

# The script will:
# - Download CPM.cmake to cmake/CPM.cmake
# - Download CPM dependencies to .cpm-cache/
# - Download USD source to OpenUSD/src/
```

**Option 2: Manual setup**
```bash
# Download USD manually
mkdir -p OpenUSD/src
cd OpenUSD/src
wget https://github.com/PixarAnimationStudios/OpenUSD/archive/refs/tags/v25.11.tar.gz
cd ../../..
```

**Build USD** (required after download):
```bash
cd OpenUSD
tar -xzf src/OpenUSD-25.11.tar.gz --strip-components=1 -C build/
mkdir -p bld && cd bld
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DPXR_BUILD_USD_USDA=ON \
  -DPXR_BUILD_USD_USDC=ON \
  -DPXR_ENABLE_PYTHON_SUPPORT=OFF \
  -DPXR_BUILD_IMAGING=OFF \
  -DPXR_BUILD_USDVIEW=OFF \
  -DPXR_BUILD_USDRENDERER=OFF \
  -DPXR_BUILD_TESTS=OFF \
  -DPXR_BUILD_EXAMPLES=OFF \
  -DCMAKE_INSTALL_PREFIX=../install
make -j$(nproc)
make install
```

### Build

```bash
cd usd_vulkan_engine

# Configure with CMake (CPM.cmake downloads remaining dependencies)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cd build && make -j$(nproc)

# Run the engine
LD_LIBRARY_PATH=../OpenUSD/install/lib:$LD_LIBRARY_PATH ./src/app/usd_vulkan_engine
```

### Build Options

```bash
# Enable validation layers (default: ON)
cmake -B build -DUSD_VULKAN_ENGINE_VALIDATION=ON ..

# Disable tests (default: ON)
cmake -B build -DUSD_VULKAN_ENGINE_BUILD_TESTS=OFF ..
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

## Dependencies

All dependencies are managed automatically by **CPM.cmake** and cached in `.cpm-cache/`:

| Dependency | Version | Source |
|------------|---------|--------|
| CPM.cmake | v0.42.0 | [github.com/cpm-cmake/CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) |
| Catch2 | v3.12.0 | [github.com/catchorg/Catch2](https://github.com/catchorg/Catch2) |
| GLM | 1.0.2 | [github.com/g-truc/glm](https://github.com/g-truc/glm) |
| Dear ImGui | v1.92.2 | [github.com/ocornut/imgui](https://github.com/ocornut/imgui) |
| spdlog | v1.16.0 | [github.com/gabime/spdlog](https://github.com/gabime/spdlog) |
| ImGuiFileDialog | v0.6.7 | [github.com/aiekick/ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog) |
| GLFW | 3.4 | [github.com/glfw/glfw](https://github.com/glfw/glfw) |
| Vulkan-Headers | v1.4.337 | [github.com/KhronosGroup/Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers) |
| Vulkan-Hpp | v1.4.337 | [github.com/KhronosGroup/Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp) |
| OpenUSD | v25.11 | [github.com/PixarAnimationStudios/OpenUSD](https://github.com/PixarAnimationStudios/OpenUSD) |

## Sample Files

The project includes sample USD files in the `data/` directory:

- **spinning_cube.usda**: Simple spinning cube demo (loaded by default)
- **walk_cycle.usda**: Humanoid skeleton with walking cycle animation
  - 19 joints (hips, spine, chest, neck, head, arms, hands, legs, feet)
  - 24-frame animation loop at 24 FPS
  - Demonstrates USD SkelAnimation and Skeleton schemas

## Cleanup

### Dependency Cleanup

To remove all downloaded and built dependencies:

```bash
# Remove CPM cached dependencies
rm -rf .cpm-cache/

# Remove USD source and build
rm -rf OpenUSD/

# Re-download dependencies
./scripts/download_dependencies.sh
```

### Build Cleanup

```bash
# From build directory:
make help-cleanup    # Show all cleanup options
make tidy            # Remove intermediate files, keep binaries
make project-clean   # Remove build artifacts, keep build dir
make distclean       # Remove entire build directory
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
├── cmake/             # CMake modules (CPM.cmake, UsdConfig.cmake)
├── scripts/           # Build and download scripts
├── OpenUSD/           # USD build directory
│   ├── src/           # USD source tarball
│   ├── build/         # USD build directory
│   └── install/       # USD install directory
├── .cpm-cache/        # CPM dependency cache (gitignored)
├── data/              # Sample USD files
│   └── walk_cycle.usda # Humanoid skeleton with walking animation
└── build/             # Build artifacts
```

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
LD_LIBRARY_PATH=../OpenUSD/install/lib:$LD_LIBRARY_PATH ctest --output-on-failure

# Run specific test suite
./tests/usd_vk_core_tests -s
./tests/usd_vk_usd_tests -s
./tests/usd_vk_render_tests -s

# Run specific test case
./tests/usd_vk_render_tests "Shader: Load from SPIR-V"

# Run shader tests only
./tests/usd_vk_render_tests "[shader]"
```

## Namespace

All project code uses the `usd_vk` namespace to avoid conflicts.

## License

MIT License
