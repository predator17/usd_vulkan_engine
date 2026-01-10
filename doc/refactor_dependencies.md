# USD Vulkan Engine - Dependencies Restructure Plan

**Document Version:** 1.1
**Date:** 2026-01-09
**Status:** AWAITING APPROVAL
**Changelog:**
- v1.1: Added GLFW and Vulkan to CPM.cmake management (fully self-contained build)

---

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [Current State Analysis](#current-state-analysis)
3. [Target Structure](#target-structure)
4. [Dependencies Version Matrix](#dependencies-version-matrix)
5. [Migration Plan](#migration-plan)
6. [Verification Procedures](#verification-procedures)
7. [Rollback Procedures](#rollback-procedures)
8. [Known Issues & Risks](#known-issues--risks)
9. [References](#references)

---

## Executive Summary

This document outlines the comprehensive restructure of the USD Vulkan Engine's dependency management system. The project will migrate from a manual `external/` directory approach to a **fully self-contained** solution using:

- **CPM.cmake** for ALL dependencies (Catch2, GLM, ImGui, spdlog, ImGuiFileDialog, GLFW, Vulkan-Headers, Vulkan-Hpp)
- **Structured USD directory** for OpenUSD (separate from CPM due to size/complexity)
- **Modern CMake practices** with imported targets and proper propagation

**Key Benefits:**
- ✅ **Zero system dependencies** - fully self-contained build
- ✅ **Cross-platform** - Windows 11, Rocky Linux 9.7/10, macOS
- Version-controlled dependencies in CMakeLists.txt
- Shared cache across projects
- Reproducible builds across all machines
- Cleaner project structure
- Edependency updates
- **No "works on my machine" issues**

**Target Platforms:**
- Windows 11 (MSVC 2022+)
- Rocky Linux 9.7/10 (GCC 11+)
- macOS 14+ (Clang/AppleClang) - optional

---

## Current State Analysis

### Current Directory Structure

```
usd_vulkan_engine/
├── external/                    # ~1.3GB, mixed content
│   ├── catch2/                  # v2.13.10 (OLD - using single_include)
│   ├── glm/                     # v0.9.9.8 (OLD)
│   ├── imgui/                   # v1.92.6 WIP (work-in-progress)
│   ├── ImGuiFileDialog/         # v0.6.9 WIP (work-in-progress)
│   ├── spdlog/                  # v1.12.0 (OLD)
│   ├── USD/
│   │   ├── OpenUSD-25.11/       # Source extracted
│   │   ├── install/             # Built binaries
│   │   └── USD-v25.11.tar.gz    # Source archive
│   ├── src/                     # ❌ BUILD ARTIFACTS (wrong location)
│   ├── tests/                   # ❌ BUILD ARTIFACTS (wrong location)
│   ├── CMakeCache.txt           # ❌ BUILD ARTIFACT
│   ├── Makefile                 # ❌ BUILD ARTIFACT
│   ├── external/                # ❌ BUILD ARTIFACT
│   └── shaders/                 # ❌ Should be in src/shaders
└── ...
```

### Issues Identified

#### Critical Issues

| Issue | Location | Severity | Impact | Status |
|-------|----------|----------|--------|--------|
| **Hardcoded Vulkan SDK path** | `src/render/CMakeLists.txt:34` | CRITICAL | Build fails on other systems | ✅ SOLVED by CPM |
| **Hardcoded Python 3.9 path** | `src/app/CMakeLists.txt:43-44` | HIGH | Fails on Python 3.10+ | ⚠️ NEEDS FIX |
| **Global `include_directories()`** | `CMakeLists.txt:73-75` | HIGH | Anti-pattern, poor hygiene | ⚠️ NEEDS FIX |
| **Global `link_directories()`** | `CMakeLists.txt:56` | HIGH | Anti-pattern, poor hygiene | ⚠️ NEEDS FIX |
| **Catch2 v2.x** | `external/catch2/` | MEDIUM | Deprecated, v3.x available | ✅ SOLVED by CPM |
| **No version constraints** | All dependencies | MEDIUM | Reproducibility risk | ✅ SOLVED by CPM |
| **GLFW system dependency** | `find_package(glfw3)` | MEDIUM | Not portable | ✅ SOLVED by CPM |

#### Code Quality Issues

1. **USD library linking** (`src/usd/CMakeLists.txt:20-31`):
   - Hardcoded `.so` filenames
   - No proper imported targets
   - Platform-specific (Linux only)

2. **CMake anti-patterns**:
   ```cmake
   # ❌ Current (anti-pattern)
   include_directories(${CMAKE_SOURCE_DIR}/src)
   link_directories(${USD_LIB_DIR})

   # ✅ Should be
   target_include_directories(mytarget PRIVATE ${CMAKE_SOURCE_DIR}/src)
   target_link_libraries(mytarget PRIVATE USD::Usd)
   ```

---

## Target Structure

### Final Directory Structure

```
usd_vulkan_engine/
├── CMakeLists.txt              # ✏️ Modified - CPM integration
├── .gitignore                  # ✏️ Modified
│
├── cmake/                      # 📁 NEW
│   ├── CPM.cmake               # 📄 CPM package manager
│   └── UsdConfig.cmake         # 📄 USD imported target
│
├── src/                        # ✅ Unchanged (source code)
│   ├── app/
│   ├── core/
│   ├── render/
│   ├── usd/
│   ├── shaders/
│   └── ...
│
├── tests/                      # ✅ Unchanged (tests)
│
├── OpenUSD/                    # 📁 NEW - USD structure
│   ├── src/                    # Source tarballs (preserve original filenames)
│   │   └── OpenUSD-25.11.tar.gz
│   ├── build/                  # Extract and compile here
│   │   ├── 25.11/              # Version-named subdirectory
│   │   │   └── <extracted contents>
│   │   └── 26.12/              # Future versions
│   └── install/                # Compiled installations
│       ├── 25.11/
│       │   ├── bin/
│       │   ├── include/
│       │   ├── lib/
│       │   └── cmake/
│       └── 26.12/
│
├── build/                      # 📁 Build output (.gitignore)
│   ├── _deps/                  # CPM/FetchContent
│   └── ...
│
└── .cpm-cache/                 # 📁 CPM cache (.gitignore)
    └── _ext/
        ├── catch2/
        ├── glm/
        ├── imgui/
        ├── spdlog/
        ├── imguifiledialog/
        ├── glfw/                 # 📄 NEW - GLFW 3.4
        ├── vulkan-headers/       # 📄 NEW - Vulkan headers
        └── vulkan-hpp/           # 📄 NEW - Vulkan C++ bindings
```

### What Gets Deleted

```
❌ DELETE:
external/catch2/           # → CPM managed
external/glm/              # → CPM managed
external/imgui/            # → CPM managed
external/ImGuiFileDialog/  # → CPM managed
external/spdlog/           # → CPM managed
external/src/              # Build artifacts
external/tests/            # Build artifacts
external/CMakeCache.txt    # Build artifacts
external/Makefile          # Build artifacts
external/external/         # Build artifacts
external/shaders/          # Move to src/shaders or keep
external/CMakeLists.txt    # No longer needed
```

---

## Dependencies Version Matrix

### Version Comparison Table

| Dependency | Current | Latest (Jan 2026) | Type | Action |
|------------|---------|-------------------|------|--------|
| **Catch2** | v2.13.10 | [v3.12.0](https://github.com/catchorg/Catch2/releases) | Compiled | ⚠️ MAJOR UPGRADE |
| **GLM** | v0.9.9.8 | [1.0.2](https://github.com/g-truc/glm/releases) | Header-only | ⚠️ MAJOR UPGRADE |
| **Dear ImGui** | v1.92.6 WIP | [v1.92.2](https://github.com/ocornut/imgui/releases) | Header-only | ✅ Use stable |
| **spdlog** | v1.12.0 | [v1.16.0](https://github.com/gabime/spdlog/releases) | Header/Compiled | ✅ UPDATE |
| **ImGuiFileDialog** | v0.6.9 WIP | [v0.6.7](https://github.com/aiekick/ImGuiFileDialog/releases) | Header-only | ✅ Use stable |
| **OpenUSD** | v25.11 | [v25.11](https://github.com/PixarAnimationStudios/OpenUSD) | Compiled | ✅ CURRENT |
| **GLFW** | System | [v3.4](https://github.com/glfw/glfw) | Compiled | ✅ VIA CPM |
| **Vulkan-Headers** | System | [v1.4.337](https://github.com/KhronosGroup/Vulkan-Headers/releases) | Header-only | ✅ VIA CPM |
| **Vulkan-Hpp** | System | [v1.4.337](https://github.com/KhronosGroup/Vulkan-Hpp/releases) | Header-only | ✅ VIA CPM |

### Breaking Changes to Address

#### Catch2 v2 → v3 Migration

Catch2 v3 is **NOT** a single-header library anymore. Key changes:

```c++
// ❌ OLD (v2.x)
#include "catch2/catch.hpp"

// ✅ NEW (v3.x)
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
```

**Migration Guide:** https://github.com/catchorg/Catch2/blob/devel/docs/migrate-v2-to-v3.md

**Test Code Changes Required:**
- All `#include "catch2/catch.hpp"` → `#include <catch2/catch_test_macros.hpp>`
- CMake integration changes (documented below)

#### GLM 0.9.9 → 1.0.0 Migration

GLM 1.0.0 has breaking changes:

- Deprecated features removed
- New quaternion types
- API changes in some extensions

**Action Required:**
- Review all GLM usage in source code
- Update any deprecated function calls
- Test math operations thoroughly

---

## Migration Plan

### Phase 1: Preparation (Pre-Execution)

#### 1.1 Create Backup Branch

```bash
git checkout -b pre-cpm-migration-backup
git add -A
git commit -m "Pre-migration backup"
git checkout main
```

#### 1.2 Create Required Directories

```bash
mkdir -p cmake
mkdir -p OpenUSD/src
mkdir -p OpenUSD/build
mkdir -p OpenUSD/install
```

#### 1.3 Move USD Structure

```bash
# Move existing USD installation
mv external/USD/install OpenUSD/install/25.11
mv external/USD/OpenUSD-25.11 OpenUSD/build/25.11
mv external/USD/USD-v25.11.tar.gz OpenUSD/src/
```

### Phase 2: File Creation

#### 2.1 Download CPM.cmake

```bash
curl -L https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/cpm.cmake \
     -o cmake/CPM.cmake
```

**SHA256 Verification (as of Jan 2026):**
```bash
# Run after download to verify
sha256sum cmake/CPM.cmake
# Expected: <checksum to be verified>
```

#### 2.2 Create `cmake/UsdConfig.cmake`

```cmake
# cmake/UsdConfig.cmake
# OpenUSD imported target configuration

if(USD_FOUND)
    return()
endif()

# USD version from directory
set(USD_VERSION "25.11")
set(USD_FOUND TRUE)

# USD installation directories
set(USD_ROOT "${CMAKE_SOURCE_DIR}/OpenUSD/install/${USD_VERSION}" CACHE PATH "USD installation path")
set(USD_INCLUDE_DIR "${USD_ROOT}/include")
set(USD_INCLUDE_DIRS ${USD_INCLUDE_DIR})
set(USD_LIB_DIR "${USD_ROOT}/lib")
set(USD_LIBRARY_DIRS ${USD_LIB_DIR})

# Find USD CMake config if available
if(EXISTS "${USD_ROOT}/cmake/pxrConfig.cmake")
    list(APPEND CMAKE_PREFIX_PATH "${USD_ROOT}/cmake")
    find_package(pxrcpp CONFIG QUIET)
else()
    # Manual imported target creation
    function(_usd_add_imported_target name)
        if(EXISTS "${USD_LIB_DIR}/lib${name}.so")
            add_library(USD::${name} SHARED IMPORTED GLOBAL)
            set_target_properties(USD::${name} PROPERTIES
                IMPORTED_LOCATION "${USD_LIB_DIR}/lib${name}.so"
                IMPORTED_NO_SONAME TRUE
                INTERFACE_INCLUDE_DIRECTORIES "${USD_INCLUDE_DIR}"
            )
        endif()
    endfunction()

    # Core USD libraries
    _usd_add_imported_target(usd)
    _usd_add_imported_target(usdGeom)
    _usd_add_imported_target(usdSdf)
    _usd_add_imported_target(usdPcp)
    _usd_add_imported_target(usdTf)
    _usd_add_imported_target(usdGf)
    _usd_add_imported_target(usdVt)
    _usd_add_imported_target(usdArch)
    _usd_add_imported_target(usdJs)
    _usd_add_imported_target(usdKind)

    # Alias for convenience
    if(TARGET USD::usd)
        add_library(USD::Usd ALIAS USD::usd)
    endif()
endif()

# RPATH for USD libraries
set(CMAKE_INSTALL_RPATH "${USD_LIB_DIR}")
set(CMAKE_BUILD_RPATH "${USD_LIB_DIR}")

message(STATUS "USD ${USD_VERSION} found at: ${USD_ROOT}")
```

#### 2.3 Update Root `CMakeLists.txt`

**Key Changes:**

```cmake
cmake_minimum_required(VERSION 3.20)
project(usd_vulkan_engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#==============================================================================
# CPM.cmake Configuration
#==============================================================================
set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cpm-cache" CACHE PATH "CPM cache directory")
set(CPM_USE_NAMED_CACHE_DIRECTORIES TRUE)  # Better IDE readability

# Download CPM.cmake if not exists
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/cmake/CPM.cmake")
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/cmake)
    message(STATUS "Downloading CPM.cmake...")
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/cpm.cmake
        ${CMAKE_SOURCE_DIR}/cmake/CPM.cmake
        EXPECTED_HASH SHA256=<checksum>
    )
endif()

include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)

#==============================================================================
# Dependencies via CPM.cmake
#==============================================================================

# Catch2 v3.12.0 - Testing framework
CPMAddPackage(
    NAME catch2
    VERSION 3.12.0
    GITHUB_REPOSITORY catchorg/Catch2
    OPTIONS "BUILD_TESTING OFF"
)
if(catch2_ADDED)
    add_library(Catch2::Catch2 ALIAS catch2)
endif()

# GLM 1.0.2 - Header-only math library
CPMAddPackage(
    NAME glm
    VERSION 1.0.2
    GITHUB_REPOSITORY g-truc/glm
)
if(glm_ADDED)
    add_library(glm::glm ALIAS glm)
endif()

# Dear ImGui v1.92.2 - Immediate mode GUI (stable release)
CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    VERSION 1.92.2
)

# spdlog v1.16.0 - Logging library
CPMAddPackage(
    NAME spdlog
    GITHUB_REPOSITORY gabime/spdlog
    VERSION 1.16.0
    OPTIONS "SPDLOG_BUILD_SHARED OFF" "SPDLOG_BUILD_TESTS OFF" "SPDLOG_INSTALL OFF"
)
if(spdlog_ADDED)
    add_library(spdlog::spdlog ALIAS spdlog)
endif()

# ImGuiFileDialog v0.6.7 - File dialog for ImGui
CPMAddPackage(
    NAME ImGuiFileDialog
    GITHUB_REPOSITORY aiekick/ImGuiFileDialog
    VERSION 0.6.7
)

# GLFW3 v3.4 - Window/input library (via CPM for self-contained builds)
CPMAddPackage(
    NAME glfw
    GITHUB_REPOSITORY glfw/glfw
    VERSION 3.4
    OPTIONS "GLFW_BUILD_TESTS OFF" "GLFW_BUILD_DOCS OFF" "GLFW_BUILD_EXAMPLES OFF"
)
if(glfw_ADDED)
    add_library(glfw ALIAS glfw)
endif()

# Vulkan-Headers v1.4.337 - Vulkan C API headers (via CPM)
CPMAddPackage(
    NAME vulkan-headers
    GITHUB_REPOSITORY KhronosGroup/Vulkan-Headers
    VERSION 1.4.337
)

# Vulkan-Hpp v1.4.337 - Vulkan C++ bindings (via CPM)
CPMAddPackage(
    NAME vulkan-hpp
    GITHUB_REPOSITORY KhronosGroup/Vulkan-Hpp
    VERSION 1.4.337
)

# Create Vulkan imported target from CPM dependencies
if(vulkan-headers_ADDED AND vulkan-hpp_ADDED)
    # Create interface target for Vulkan headers
    add_library(Vulkan::Headers INTERFACE IMPORTED GLOBAL)
    target_include_directories(Vulkan::Headers INTERFACE
        ${vulkan-headers_SOURCE_DIR}/include
        ${vulkan-hpp_SOURCE_DIR}
    )

    # Create alias for compatibility
    add_library(Vulkan::Vulkan ALIAS Vulkan::Headers)

    message(STATUS "Vulkan ${vulkan-headers_VERSION} via CPM")
endif()

#==============================================================================
# USD - Local Build (Not managed by CPM)
#==============================================================================
include(${CMAKE_SOURCE_DIR}/cmake/UsdConfig.cmake)

#==============================================================================
# Project Subdirectories
#==============================================================================
add_subdirectory(src)

if(USD_VULKAN_ENGINE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

#==============================================================================
# Cleanup Targets (existing - keep unchanged)
#==============================================================================
# ... existing cleanup targets remain here ...
```

#### 2.4 Update `src/usd/CMakeLists.txt`

```cmake
# USD library

set(USD_SOURCES
    stage.cpp
    mesh.cpp
)

set(USD_HEADERS
    stage.hpp
    mesh.hpp
)

add_library(usd_vk_usd ${USD_SOURCES})
target_include_directories(usd_vk_usd PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_vk_usd PUBLIC usd_vk_core)

# USD libraries - use modern imported target
if(TARGET USD::Usd)
    # Modern approach with imported target
    target_link_libraries(usd_vk_usd PUBLIC USD::Usd)
elseif(EXISTS "${USD_LIB_DIR}")
    # Fallback to manual linking (backward compatibility)
    target_link_libraries(usd_vk_usd PUBLIC
        ${USD_LIB_DIR}/libusd.so
        ${USD_LIB_DIR}/libusdGeom.so
        ${USD_LIB_DIR}/libusd_sdf.so
        ${USD_LIB_DIR}/libusd_pcp.so
        ${USD_LIB_DIR}/libusd_tf.so
        ${USD_LIB_DIR}/libusd_gf.so
        ${USD_LIB_DIR}/libusd_vt.so
        ${USD_LIB_DIR}/libusd_arch.so
        ${USD_LIB_DIR}/libusd_js.so
        ${USD_LIB_DIR}/libusd_kind.so
        Threads::Threads
    )
    target_include_directories(usd_vk_usd PUBLIC ${USD_INCLUDE_DIR})
endif()
```

#### 2.5 Update `src/render/CMakeLists.txt`

**Remove hardcoded Vulkan SDK path:**

```cmake
# BEFORE (line 34):
# "/mnt/wd_hdd_18t/rd/dependencies/vulkan/1.3.290.0/x86_64/bin"

# AFTER:
find_program(GLSLANG_VALIDATOR glslangValidator
    PATHS
        "/usr/bin"
        "/usr/local/bin"
        "$ENV{VULKAN_SDK}/Bin"
        "/opt/VulkanSDK"
)
```

#### 2.6 Update `tests/CMakeLists.txt`

```cmake
# Use CPM Catch2 target
if(TARGET Catch2::Catch2)
    add_library(Catch2::Catch2 ALIAS catch2)
endif()

function(add_usd_vk_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name}
        PRIVATE
        usd_vk_core
        Catch2::Catch2
        Vulkan::Vulkan
        glm::glm
        spdlog::spdlog
        Threads::Threads
    )
    # ... rest of function
endfunction()

function(add_usd_vk_render_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name}
        PRIVATE
        usd_vk_core
        usd_vk_render
        Catch2::Catch2
        Vulkan::Vulkan
        glm::glm
        spdlog::spdlog
        Threads::Threads
        glfw  # From CPM, not system package
    )
    # ... rest of function
endfunction()
```

#### 2.7 Update `.gitignore`

```gitignore
# Build outputs
build/
build-*/
compile_commands.json

# CPM cache
.cpm-cache/

# CMake generated files
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
Makefile
*.cmake
!cmake/CPM.cmake
!cmake/UsdConfig.cmake

# IDE
.vscode/
.idea/
*.swp
*.swo
*~

# OS
.DS_Store
Thumbs.db

# Shader binaries
*.spv
```

#### 2.8 Note on Vulkan Loader (IMPORTANT)

**The CPM-managed Vulkan-Headers and Vulkan-Hpp provide HEADERS ONLY.**

The actual Vulkan **loader** (shared library) must still be available on the system:

| Platform | Loader Library | Source |
|----------|---------------|--------|
| **Linux** | `libvulkan.so.1` | Graphics driver (NVIDIA/AMD/Intel) |
| **Windows** | `vulkan-1.dll` | Graphics driver or Vulkan SDK |
| **macOS** | `libvulkan.dylib` | MoltenVK |

**This is intentional** - The loader must match the installed GPU driver and cannot be pre-bundled.

**What this means for you:**
- Users still need graphics drivers with Vulkan support installed
- The build will work cross-platform as long as drivers are present
- Headers are version-controlled and reproducible via CPM
- No SDK installation needed for compilation (only drivers)

### Phase 3: Test Code Migration

#### 3.1 Update Test Includes

All test files need to be updated from Catch2 v2 to v3:

```c++
// ❌ OLD
#include "catch2/catch.hpp"

// ✅ NEW
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
```

**Files to update:**
- `tests/main.cpp`
- All test files in `tests/core/`
- All test files in `tests/render/`
- All test files in `tests/usd/`

### Phase 4: Build Verification

#### 4.1 Clean Build

```bash
# Remove all build artifacts
rm -rf build/
rm -rf .cpm-cache/

# Fresh build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

#### 4.2 Run Tests

```bash
cd build
ctest --output-on-failure
```

#### 4.3 Run Application

```bash
cd build
./src/app/usd_vulkan_engine
```

### Phase 5: Cleanup (After Successful Verification)

```bash
# Only run after successful build and test!

# Remove old external dependencies
rm -rf external/catch2
rm -rf external/glm
rm -rf external/imgui
rm -rf external/ImGuiFileDialog
rm -rf external/spdlog

# Remove build artifacts from external
rm -rf external/src
rm -rf external/tests
rm -f external/CMakeCache.txt
rm -f external/Makefile
rm -rf external/external
rm -f external/CMakeLists.txt

# Remove old shaders (if moved elsewhere)
# rm -rf external/shaders  # Only if confirmed safe

# Remove empty external directory
rmdir external 2>/dev/null || true
```

### Phase 6: Commit Changes

```bash
git add -A
git commit -m "Migrate to CPM.cmake dependency management

- Add CPM.cmake for small/medium dependencies
- Restructure OpenUSD to OpenUSD/ with src/build/install
- Upgrade Catch2 from v2.13.10 to v3.12.0
- Upgrade GLM from v0.9.9.8 to 1.0.2
- Upgrade spdlog from v1.12.0 to v1.16.0
- Use stable ImGui v1.92.2 and ImGuiFileDialog v0.6.7
- Remove hardcoded paths from CMakeLists.txt
- Add modern imported targets for USD
- Update .gitignore for CPM cache

BREAKING CHANGES:
- Catch2 v3 requires different includes
- GLM 1.0.0 has API changes
- Tests updated for Catch2 v3 syntax"
```

---

## Verification Procedures

### Pre-Migration Checklist

- [ ] Backup branch created (`pre-cpm-migration-backup`)
- [ ] All current tests passing
- [ ] Current application builds and runs successfully
- [ ] Disk space available (~2GB for build artifacts)
- [ ] Internet connection available (for CPM downloads)
- [ ] Python 3 available (for USD build if needed)
- [ ] CMake 3.20+ installed
- [ ] C++20 compatible compiler available

### Post-Migration Verification

#### Step 1: CMake Configuration

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Verify output contains:
# ✅ CPM: catch2 added
# ✅ CPM: glm added
# ✅ CPM: spdlog added
# ✅ CPM: imgui added
# ✅ CPM: ImGuiFileDialog added
# ✅ CPM: glfw added
# ✅ CPM: vulkan-headers added
# ✅ CPM: vulkan-hpp added
# ✅ USD 25.11 found at: <path>
# ✅ Vulkan <version> via CPM
```

#### Step 2: Build Verification

```bash
cmake --build build -j$(nproc)

# Expected:
# ✅ All targets compile without errors
# ✅ No linking errors
# ✅ All CPM dependencies found
```

#### Step 3: Test Verification

```bash
cd build
ctest --output-on-failure --verbose

# Expected:
# ✅ All core tests pass
# ✅ All render tests pass
# ✅ All USD tests pass (if USD available)
```

#### Step 4: Application Verification

```bash
# Run the application
./src/app/usd_vulkan_engine

# Verify:
# ✅ Application launches
# ✅ Window opens
# ✅ Rendering works
# ✅ UI (ImGui) displays correctly
# ✅ File dialog works
# ✅ No console errors/warnings
```

#### Step 5: Dependency Version Verification

```bash
# Check CPM cache
ls -la .cpm-cache/_ext/

# Expected:
# catch2/<version>/
# glm/<version>/
# spdlog/<version>/
# imgui/<version>/
# imguifiledialog/<version>/
# glfw/<version>/
# vulkan-headers/<version>/
# vulkan-hpp/<version>/

# Check USD structure
ls -la OpenUSD/

# Expected:
# src/OpenUSD-25.11.tar.gz
# build/25.11/
# install/25.11/
```

### Performance Verification

```bash
# Measure build time (first build - downloads dependencies)
time cmake --build build -j$(nproc)

# Clean and rebuild (should be faster with cache)
rm -rf build/
time cmake -B build -DCMAKE_BUILD_TYPE=Release
time cmake --build build -j$(nproc)
```

---

## Rollback Procedures

### Option 1: Git Reset (If Not Committed)

```bash
# Reset all changes
git reset --hard HEAD
git clean -fd

# Restore external directory
git checkout main -- external/
```

### Option 2: Git Checkout (If Committed)

```bash
# Checkout backup branch
git checkout pre-cpm-migration-backup

# Or reset to before migration
git reset --hard <commit-hash-before-migration>
```

### Option 3: Manual Rollback

```bash
# Restore USD structure
mv OpenUSD/install/25.11 external/USD/install
mv OpenUSD/build/25.11 external/USD/OpenUSD-25.11
mv OpenUSD/src/OpenUSD-25.11.tar.gz external/USD/
mv external/USD/install external/USD/OpenUSD-25.11

# Restore CMakeLists.txt from git
git checkout main -- CMakeLists.txt
git checkout main -- src/*/CMakeLists.txt
git checkout main -- tests/CMakeLists.txt
git checkout main -- .gitignore

# Restore old external dependencies (if still present in git)
# Or re-download from original sources
```

### Rollback Verification

```bash
# After rollback, verify:
cmake -B build
cmake --build build -j$(nproc)
ctest --output-on-failure
./src/app/usd_vulkan_engine
```

---

## Known Issues & Risks

### Breaking Changes

| Issue | Impact | Mitigation |
|-------|--------|------------|
| Catch2 v2 → v3 | All test includes need update | Automated find/replace |
| GLM 0.9 → 1.0 | Potential API changes | Review GLM usage |
| USD RPATH | May need adjustment for new paths | Test on target system |

### Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| CPM download fails | Low | High | Use cached CPM.cmake |
| Catch2 v3 migration issues | Medium | Medium | Follow migration guide |
| GLM 1.0 API breakage | Low | Medium | Test thoroughly |
| Network dependency for first build | Low | Medium | Allow offline via pre-download |
| Build time increase (first build) | High | Low | Document expected times |
| Cache compatibility | Low | Low | Use project-local cache |

### Platform-Specific Concerns

#### Windows 11
- **Compiler**: MSVC 2022+ (Visual Studio 17.0+)
- **USD Libraries**: `.dll` and `.lib` files
- **Path Separators**: CMake handles automatically
- **GLFW**: Requires `GLFW_BUILD_WIN32` option
- **Python**: Find via `find_package(Python3 COMPONENTS Interpreter)`

#### Rocky Linux 9.7/10
- **Compiler**: GCC 11+ or Clang
- **USD Libraries**: `.so` shared libraries
- **RPATH Configuration**: Critical for USD library loading
- **Package Dependencies**: Still need system Vulkan **loader** (libvulkan.so) from graphics driver
- **X11/Wayland**: GLFW 3.4 supports both runtime

#### macOS 14+ (Optional)
- **Compiler**: AppleClang (Xcode 15.0+)
- **USD Libraries**: `.dylib` shared libraries
- **Framework Linking**: May need special handling for Cocoa
- **Vulkan**: MoltenVK for translation to Metal

### Important Note on Vulkan

The CPM-managed **Vulkan-Headers** and **Vulkan-Hpp** provide **headers only**. The actual Vulkan **loader** (libvulkan.so on Linux, vulkan-1.dll on Windows) must still be provided by:

1. **Graphics Driver** - Most common source
2. **Vulkan SDK** - For additional tools (validation layers, shaders)

**This is intentional** - the loader must match the GPU driver and is platform-specific.

---

## References

### Official Documentation

- **[CPM.cmake GitHub](https://github.com/cpm-cmake/CPM.cmake)** - CPM package manager
- **[OpenUSD Build Guide](https://github.com/PixarAnimationStudios/OpenUSD/blob/dev/BUILDING.md)** - USD build instructions
- **[OpenUSD Quick Start](https://zread.ai/PixarAnimationStudios/OpenUSD/2-quick-start)** - USD getting started
- **[CMake target_link_libraries](https://cmake.org/cmake/help/latest/command/target_link_libraries.html)** - Official CMake docs
- **[CMake Importing Guide](https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html)** - Imported targets
- **[Catch2 v3 Migration](https://github.com/catchorg/Catch2/blob/devel/docs/migrate-v2-to-v3.md)** - v2 to v3 guide

### Dependency Repositories

- **[Catch2](https://github.com/catchorg/Catch2)** - Testing framework
- **[GLM](https://github.com/g-truc/glm)** - OpenGL Mathematics
- **[Dear ImGui](https://github.com/ocornut/imgui)** - Immediate mode GUI
- **[spdlog](https://github.com/gabime/spdlog)** - Fast C++ logging
- **[ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog)** - File dialog for ImGui
- **[GLFW](https://github.com/glfw/glfw)** - Window/input library
- **[Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers)** - Vulkan C API headers
- **[Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp)** - Vulkan C++ bindings
- **[OpenUSD](https://github.com/PixarAnimationStudios/OpenUSD)** - Universal Scene Description

### Best Practices

- **[Modern CMake Guide](https://cliutils.gitlab.io/modern-cmake/modern-cmake.pdf)** - CMake best practices
- **[C++20 Features](https://en.cppreference.com/w/cpp/20)** - C++20 reference
- **[Vulkan 1.4 Specification](https://docs.vulkan.org/spec/latest/)** - Vulkan documentation

---

## Appendix A: Command Summary

### Complete Migration Script

```bash
#!/bin/bash
set -e  # Exit on error

echo "=== USD Vulkan Engine - Dependencies Migration ==="
echo ""

# Phase 1: Backup
echo "Phase 1: Creating backup..."
git checkout -b pre-cpm-migration-backup
git add -A
git commit -m "Pre-migration backup"
git checkout main

# Phase 2: Create directories
echo "Phase 2: Creating directory structure..."
mkdir -p cmake
mkdir -p OpenUSD/src
mkdir -p OpenUSD/build
mkdir -p OpenUSD/install

# Phase 3: Move USD
echo "Phase 3: Moving USD structure..."
if [ -d "external/USD" ]; then
    mv external/USD/install OpenUSD/install/25.11
    mv external/USD/OpenUSD-25.11 OpenUSD/build/25.11
    mv external/USD/USD-v25.11.tar.gz OpenUSD/src/
fi

# Phase 4: Download CPM.cmake
echo "Phase 4: Downloading CPM.cmake..."
curl -L https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/cpm.cmake \
     -o cmake/CPM.cmake

# Phase 5: Clean build
echo "Phase 5: Cleaning build artifacts..."
rm -rf build/
rm -rf .cpm-cache/

# Phase 6: Configure
echo "Phase 6: Configuring with new CMakeLists.txt..."
# Note: CMakeLists.txt must be updated first (see document)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Phase 7: Build
echo "Phase 7: Building..."
cmake --build build -j$(nproc)

# Phase 8: Test
echo "Phase 8: Running tests..."
cd build
ctest --output-on-failure
cd ..

echo ""
echo "=== Migration Complete ==="
echo "Please verify the application runs correctly before cleanup."
```

---

## Appendix B: Test File Conversion

### Catch2 v2 → v3 Include Changes

```c++
// OLD (v2.x) - REMOVE
// #include "catch2/catch.hpp"

// NEW (v3.x) - ADD
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>  // If using benchmarks
#include <catch2/generators/catch_generators.hpp>  // If using generators
#include <catch2/matchers/catch_matchers.hpp>      // If using matchers
```

### MAIN.CPP Changes

```c++
// OLD (v2.x)
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

// NEW (v3.x)
#include <catch2/catch_session.hpp>

int main(int argc, char* argv[]) {
    return Catch::Session().run(argc, argv);
}
```

---

## Approval Required

Before execution, please confirm:

- [ ] I have reviewed this document in its entirety
- [ ] I understand the breaking changes required
- [ ] I have a backup of my work
- [ ] I am ready to proceed with the migration

**Signature:** ___________________
**Date:** ___________________

---

*Document End*
