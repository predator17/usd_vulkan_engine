# USD Vulkan Engine - TODO

## High Priority

### USD Scene Loading Implementation

**Status**: API ready, parser uses hardcoded demo (spinning cube)

**Issue**: When loading any USD file (e.g., `data/walk_cycle.usda`), the engine displays
a hardcoded spinning cube instead of the actual scene content.

**Root Cause**: The `Scene::loadUSDArchive()` function in `src/scene/scene.cpp:43-47`
is not implemented - it just logs an error and returns false. The fallback
`extractMeshData()` function creates a hardcoded cube.

**Required Implementation**:

1. **USD Stage Loading** (`src/scene/scene.cpp`)
   - Use USD API to open stage: `UsdStage::Open()`
   - Traverse scene hierarchy using `UsdPrimRange`
   - Detect and handle Xformable, Mesh, and Skeleton prims

2. **Mesh Geometry Extraction** (`src/scene/scene.cpp`)
   - Extract points (vertices) from `UsdGeomMesh`
   - Extract face vertex indices
   - Extract normals (compute if absent)
   - Extract primvar attributes (color, texcoord)
   - Handle varying, vertex, and faceVarying interpolation

3. **Transform Extraction** (`src/scene/scene.cpp`)
   - Extract local-to-world transform from `UsdGeomXformable`
   - Handle transform operations: translate, rotate, scale
   - Build transform hierarchy for nested prims

4. **Skeleton and Animation Support** (Future)
   - Parse `UsdSkelSkeleton` topology
   - Extract `UsdSkelAnimation` joint transforms
   - Implement skinning (optional, complex)

**Reference Files**:
- `src/scene/scene.cpp` - Current implementation (lines 43-141)
- `src/scene/scene.hpp` - Scene interface
- `data/walk_cycle.usda` - Test file with skeleton and animation
- `data/spinning_cube.usda` - Simple test case

**USD API Examples**:
```cpp
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/xformable.h>

using namespace pxr;

// Open stage
UsdStageRefPtr stage = UsdStage::Open(filePath);

// Traverse prims
for (UsdPrim prim : stage->Traverse()) {
    if (UsdGeomMesh mesh = UsdGeomMesh(prim)) {
        // Extract mesh data...
    }
}
```

**Test Cases**:
1. Load `data/spinning_cube.usda` - should display cube
2. Load `data/walk_cycle.usda` - should display skeleton hierarchy
3. Animation playback should animate joints

---

## Medium Priority

### Shader Improvements
- [ ] Add PBR material system (metalness, roughness maps)
- [ ] Add texture sampling support
- [ ] Implement skeletal animation vertex shader

### UI Enhancements
- [ ] Add skeleton hierarchy viewer for walk_cycle.usda
- [ ] Add timeline scrubbing for animation
- [ ] Add joint transform visualization

### Performance
- [ ] Frustum culling
- [ ] Level of detail (LOD) system
- [ ] Multi-threaded scene loading

---

## Low Priority

### Platform Support
- [ ] Windows build fixes
- [ ] macOS build support (MoltenVK)

### Documentation
- [ ] API documentation (Doxygen)
- [ ] Architecture diagrams
- [ ] Contribution guidelines

### Testing
- [ ] USD loading integration tests
- [ ] Animation playback tests
- [ ] Performance benchmarks
