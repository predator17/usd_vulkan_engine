# Dependencies Download Summary

**Date:** 2026-01-09
**Proxy:** http://127.0.0.1:8118

---

## What Will Be Downloaded

### Quick Reference Table

| # | Dependency | Version | Size (approx) | Official Source |
|---|------------|---------|---------------|-----------------|
| 1 | CPM.cmake | v0.42.0 | ~50 KB | [github.com/cpm-cmake/CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) |
| 2 | Catch2 | v3.12.0 | ~2 MB | [github.com/catchorg/Catch2](https://github.com/catchorg/Catch2) |
| 3 | GLM | 1.0.2 | ~1 MB | [github.com/g-truc/glm](https://github.com/g-truc/glm) |
| 4 | Dear ImGui | v1.92.2 | ~2 MB | [github.com/ocornut/imgui](https://github.com/ocornut/imgui) |
| 5 | spdlog | v1.16.0 | ~1 MB | [github.com/gabime/spdlog](https://github.com/gabime/spdlog) |
| 6 | ImGuiFileDialog | v0.6.7 | ~500 KB | [github.com/aiekick/ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog) |
| 7 | GLFW | 3.4 | ~3 MB | [github.com/glfw/glfw](https://github.com/glfw/glfw) |
| 8 | Vulkan-Headers | v1.4.337 | ~2 MB | [github.com/KhronosGroup/Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers) |
| 9 | Vulkan-Hpp | v1.4.337 | ~3 MB | [github.com/KhronosGroup/Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp) |
| 10 | OpenUSD | v25.11 | ~150 MB | [github.com/PixarAnimationStudios/OpenUSD](https://github.com/PixarAnimationStudios/OpenUSD) |

**Total Download Size:** ~165 MB

---

## Direct Download URLs

### 1. CPM.cmake (Package Manager)
```bash
https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.42.0/CPM.cmake
```
- **Destination:** `cmake/CPM.cmake`
- **Purpose:** CMake dependency manager

### 2. Catch2 (Testing Framework)
```bash
https://github.com/catchorg/Catch2/archive/refs/tags/v3.12.0.tar.gz
```
- **Destination:** `.cpm-cache/_ext/` (managed by CPM)
- **Purpose:** Unit testing framework

### 3. GLM (Math Library)
```bash
https://github.com/g-truc/glm/archive/refs/tags/1.0.2.tar.gz
```
- **Destination:** `.cpm-cache/_ext/` (managed by CPM)
- **Purpose:** OpenGL mathematics (header-only)

### 4. Dear ImGui (UI Framework)
```bash
https://github.com/ocornut/imgui/archive/refs/tags/v1.92.2.tar.gz
```
- **Destination:** `.cpm-cache/_ext/` (managed by CPM)
- **Purpose:** Immediate mode GUI

### 5. spdlog (Logging)
```bash
https://github.com/gabime/spdlog/archive/refs/tags/v1.16.0.tar.gz
```
- **Destination:** `.cpm-cache/_ext/` (managed by CPM)
- **Purpose:** Fast C++ logging library

### 6. ImGuiFileDialog (File Dialog)
```bash
https://github.com/aiekick/ImGuiFileDialog/archive/refs/tags/v0.6.7.tar.gz
```
- **Destination:** `.cpm-cache/_ext/` (managed by CPM)
- **Purpose:** File dialog for ImGui

### 7. GLFW (Window/Input)
```bash
https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz
```
- **Destination:** `.cpm-cache/_ext/` (managed by CPM)
- **Purpose:** Window and input handling

### 8. Vulkan-Headers (Vulkan C API)
```bash
https://github.com/KhronosGroup/Vulkan-Headers/archive/refs/tags/v1.4.337.tar.gz
```
- **Destination:** `.cpm-cache/_ext/` (managed by CPM)
- **Purpose:** Vulkan C API headers

### 9. Vulkan-Hpp (Vulkan C++ API)
```bash
https://github.com/KhronosGroup/Vulkan-Hpp/archive/refs/tags/v1.4.337.tar.gz
```
- **Destination:** `.cpm-cache/_ext/` (managed by CPM)
- **Purpose:** Vulkan C++ bindings

### 10. OpenUSD (USD Scene Description)
```bash
https://github.com/PixarAnimationStudios/OpenUSD/archive/refs/tags/v25.11.tar.gz
```
- **Destination:** `OpenUSD/src/OpenUSD-25.11.tar.gz`
- **Purpose:** Universal Scene Description

---

## Verification Commands

After download, verify all files:

```bash
# Check CPM.cmake
ls -lh cmake/CPM.cmake

# Check CPM cache contents
ls -lh .cpm-cache/_ext/

# Check USD source
ls -lh OpenUSD/src/OpenUSD-25.11.tar.gz
```

---

## Proxy Configuration

The download script uses:
- **HTTP Proxy:** `http://127.0.0.1:8118`
- **HTTPS Proxy:** `http://127.0.0.1:8118`

To override:
```bash
export HTTP_PROXY=""
export HTTPS_PROXY=""
./scripts/download_dependencies.sh
```
