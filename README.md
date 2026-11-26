# USD Vulkan Engine

A C++ / Vulkan rendering study project built around real-time graphics infrastructure, camera and UI controls, and a small USD integration layer.

The project is intended for graphics-programming study and technical discussion. It is **not yet a general-purpose USD viewer**.

## What is implemented

- Vulkan instance, device, queue and debug-messenger setup
- Swapchain, depth buffer, render pass and framebuffers
- Vertex, index and uniform buffers
- Descriptor set layout and descriptor pool management
- Solid-fill and wireframe graphics pipelines
- Command-buffer recording and synchronization
- SPIR-V shader loading and shader hot reload
- Orbit camera with rotate, pan and zoom controls
- Dear ImGui integration, playback controls and statistics display
- Spinning-cube demo with animation controls
- Catch2 test suite with more than 400 assertions

## Current boundaries

- USD integration exposes `Stage` and `Mesh` interfaces, but the current parser path uses **hardcoded demonstration data**. Do not expect arbitrary `.usd`, `.usda`, `.usdc` or `.usdz` files to load correctly.
- The currently documented and tested environment is **WSL2 Rocky Linux 10 on Windows 11**. Native Windows support is still being worked on.
- `src/animation` and `src/material` contain placeholders rather than complete systems.

These boundaries are part of the project documentation, not hidden limitations. Contributions that improve them are welcome.

## Technology stack

| Area | Dependency |
| --- | --- |
| Language | C++20 |
| Rendering | Vulkan-Hpp 1.4.337 |
| Scene format | Pixar OpenUSD 25.11 |
| Math | GLM 1.0.2 |
| Window | GLFW 3.4 |
| UI | Dear ImGui 1.92.2, ImGuiFileDialog 0.6.7 |
| Logging | spdlog 1.16.0 |
| Testing | Catch2 3.12.0 |
| Build | CMake 3.20+, CPM.cmake |

## Quick start

### Prerequisites

- A C++20 compiler: GCC 11+, Clang 13+ or MSVC 2022+
- CMake 3.20+
- Git
- A Vulkan-capable GPU and a driver that provides the Vulkan runtime
- WSL2 Rocky Linux 10 on Windows 11 for the currently documented setup

Install the WSL system dependencies:

```bash
sudo dnf install tbb-devel libX11-devel libXext-devel libXcursor-devel \
  libXrandr-devel libXinerama-devel libXi-devel libXrender-devel \
  libxkbcommon-devel wayland-devel
```

### 1. Download the CPM dependencies

```bash
./scripts/download_dependencies.sh
```

The script downloads the graphics and test dependencies into `.cpm-cache/`.

### 2. Download and build OpenUSD

```bash
./scripts/setup_usd.sh
```

To select another OpenUSD release, pass its version explicitly:

```bash
./scripts/setup_usd.sh v25.11
```

The build creates versioned directories under `OpenUSD/` and compatibility symlinks under `OpenUSD/install/`. GUI tools such as `usda` and `usdview` are disabled so that the core libraries can be built in a headless environment.

### 3. Configure, build and run

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
LD_LIBRARY_PATH=./OpenUSD/install/lib:$LD_LIBRARY_PATH ./build/src/app/usd_vulkan_engine
```

Validation layers are enabled by default:

```bash
cmake -B build -DUSD_VULKAN_ENGINE_VALIDATION=ON .
```

To disable tests:

```bash
cmake -B build -DUSD_VULKAN_ENGINE_BUILD_TESTS=OFF .
```

## Controls

| Input | Action |
| --- | --- |
| `Esc` | Exit |
| `Space` | Play or pause animation |
| `Left` / `Right` | Previous or next frame |
| Left mouse drag | Orbit |
| Right mouse drag | Pan |
| Mouse wheel | Zoom |

The UI also provides a file dialog, wireframe toggle and statistics display. The file dialog does not remove the USD parser boundary described above.

## Sample data

The `data/` directory contains:

- `spinning_cube.usda`: a simple animated cube used by the demo.
- `walk_cycle.usda`: a 19-joint humanoid skeleton with a 24-frame walking loop, used to study `UsdSkelAnimation` and skeleton schemas.

## Tests

```bash
LD_LIBRARY_PATH=./OpenUSD/install/lib:$LD_LIBRARY_PATH ctest --test-dir build --output-on-failure
```

Run an individual suite or test case:

```bash
./build/tests/usd_vk_core_tests -s
./build/tests/usd_vk_usd_tests -s
./build/tests/usd_vk_render_tests -s
./build/tests/usd_vk_render_tests "Shader: Load from SPIR-V"
./build/tests/usd_vk_render_tests "[shader]"
```

## Architecture

```text
src/
├── app/          Application entry point and main loop
├── core/         Color, bounds, transforms, timing, logging and assertions
├── usd/          USD stage and mesh integration boundary
├── render/       Vulkan context, resources, pipelines and renderer
├── scene/        Mesh data and time-based scene updates
├── camera/       Perspective orbit camera
├── shaders/      GLSL sources and generated SPIR-V
├── ui/           ImGui integration and application controls
├── animation/    Placeholder for future animation work
└── material/     Placeholder for future material work
```

The Vulkan renderer owns device resources, swapchain and depth handling, descriptor and buffer management, solid and wireframe pipelines, shader reload, command recording and synchronization. The USD layer currently provides a small interface and demonstration data rather than a complete scene importer.

## Documentation

- `doc/beginners_guide.md`: guided introduction to the project and graphics concepts.
- `doc/cpp_techniques.md`: C++ techniques used by the implementation.
- `doc/research.md`: research notes and design references.
- `doc/refactor_dependencies.md`: dependency and refactoring notes.
- `doc/TODO.md`: remaining work and known tasks.

## Contributing

Please read `CONTRIBUTING.md` before opening a pull request. Useful contributions include real USD scene loading, native Windows support, animation and material systems, tests, documentation and reproducible build improvements.

## License

Released under the MIT License. See `LICENSE`.
