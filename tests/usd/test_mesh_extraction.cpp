#include <catch2/catch.hpp>
#include <usd/mesh.hpp>
#include <usd/stage.hpp>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/geom/mesh.h>
#include <fstream>
#include <glm/glm.hpp>

using namespace usd_ve;

TEST_CASE("Mesh: Default construction", "[usd][mesh]") {
    Mesh mesh;

    SECTION("Initially has no vertices") {
        REQUIRE(mesh.getVertexCount() == 0);
    }

    SECTION("Initially has no indices") {
        REQUIRE(mesh.getIndexCount() == 0);
    }

    SECTION("Initially has no normals") {
        REQUIRE(mesh.getNormalCount() == 0);
    }

    SECTION("Initially has no UVs") {
        REQUIRE(mesh.getUVCount() == 0);
    }
}

TEST_CASE("Mesh: Extract from null prim", "[usd][mesh]") {
    Mesh mesh;
    pxr::UsdPrim nullPrim;

    SECTION("Extracting from null prim fails") {
        REQUIRE_FALSE(mesh.extractFromPrim(nullPrim));
    }

    SECTION("Mesh data remains empty after failed extract") {
        mesh.extractFromPrim(nullPrim);
        REQUIRE(mesh.getVertexCount() == 0);
        REQUIRE(mesh.getIndexCount() == 0);
    }
}

TEST_CASE("Mesh: Create and extract from simple mesh", "[usd][mesh]") {
    // Create a simple USD file with a triangle mesh
    std::string tempFile = "/tmp/test_simple_mesh.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "(\n";
    file << "    defaultPrim = \"Mesh\"\n";
    file << ")\n";
    file << "def Mesh \"Mesh\" {\n";
    file << "    point3f[] points = [(0, 0, 0), (1, 0, 0), (0, 1, 0)]\n";
    file << "    int[] faceVertexCounts = [3]\n";
    file << "    int[] faceVertexIndices = [0, 1, 2]\n";
    file << "}\n";
    file.close();

    // Load the stage
    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    REQUIRE(stage);

    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Mesh"));
    REQUIRE(prim);

    Mesh mesh;

    SECTION("Extract mesh data from valid prim") {
        bool result = mesh.extractFromPrim(prim);
        REQUIRE(result);
    }

    SECTION("Extracts correct vertex count") {
        mesh.extractFromPrim(prim);
        REQUIRE(mesh.getVertexCount() == 3);
    }

    SECTION("Extracts correct index count") {
        mesh.extractFromPrim(prim);
        REQUIRE(mesh.getIndexCount() == 3);
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("Mesh: Extract quad mesh", "[usd][mesh]") {
    std::string tempFile = "/tmp/test_quad_mesh.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Mesh \"Mesh\" {\n";
    file << "    point3f[] points = [\n";
    file << "        (0, 0, 0), (1, 0, 0), (1, 1, 0), (0, 1, 0)\n";
    file << "    ]\n";
    file << "    int[] faceVertexCounts = [4]\n";
    file << "    int[] faceVertexIndices = [0, 1, 2, 3]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Mesh"));

    Mesh mesh;
    mesh.extractFromPrim(prim);

    SECTION("Quad has 4 vertices") {
        REQUIRE(mesh.getVertexCount() == 4);
    }

    SECTION("Quad has 4 indices") {
        REQUIRE(mesh.getIndexCount() == 4);
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("Mesh: Extract multiple faces", "[usd][mesh]") {
    std::string tempFile = "/tmp/test_multi_face.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Mesh \"Mesh\" {\n";
    file << "    point3f[] points = [\n";
    file << "        (0, 0, 0), (1, 0, 0), (0, 1, 0),  // Triangle 1\n";
    file << "        (2, 0, 0), (3, 0, 0), (2, 1, 0)   // Triangle 2\n";
    file << "    ]\n";
    file << "    int[] faceVertexCounts = [3, 3]\n";
    file << "    int[] faceVertexIndices = [0, 1, 2, 3, 4, 5]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Mesh"));

    Mesh mesh;
    mesh.extractFromPrim(prim);

    SECTION("Two triangles have 6 total vertices") {
        REQUIRE(mesh.getVertexCount() == 6);
    }

    SECTION("Two triangles have 6 indices") {
        REQUIRE(mesh.getIndexCount() == 6);
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("Mesh: Extract normals", "[usd][mesh]") {
    std::string tempFile = "/tmp/test_normals.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Mesh \"Mesh\" {\n";
    file << "    point3f[] points = [(0, 0, 0), (1, 0, 0), (0, 1, 0)]\n";
    file << "    int[] faceVertexCounts = [3]\n";
    file << "    int[] faceVertexIndices = [0, 1, 2]\n";
    file << "    normal3f[] normals = [(0, 0, 1), (0, 0, 1), (0, 0, 1)]\n";
    file << "    uniform token normalInterpolation = \"vertex\"\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Mesh"));

    Mesh mesh;
    mesh.extractFromPrim(prim);

    SECTION("Extracts normals count") {
        REQUIRE(mesh.getNormalCount() == 3);
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("Mesh: Extract UVs", "[usd][mesh]") {
    std::string tempFile = "/tmp/test_uvs.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Mesh \"Mesh\" {\n";
    file << "    point3f[] points = [(0, 0, 0), (1, 0, 0), (0, 1, 0)]\n";
    file << "    int[] faceVertexCounts = [3]\n";
    file << "    int[] faceVertexIndices = [0, 1, 2]\n";
    file << "    float2[] primvars:st = [(0, 0), (1, 0), (0, 1)]\n";
    file << "    uniform token primvars:st interpolation = \"vertex\"\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Mesh"));

    Mesh mesh;
    mesh.extractFromPrim(prim);

    SECTION("Extracts UV count") {
        REQUIRE(mesh.getUVCount() == 3);
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("Mesh: Get vertex data", "[usd][mesh]") {
    std::string tempFile = "/tmp/test_vertex_data.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Mesh \"Mesh\" {\n";
    file << "    point3f[] points = [(1, 2, 3), (4, 5, 6), (7, 8, 9)]\n";
    file << "    int[] faceVertexCounts = [3]\n";
    file << "    int[] faceVertexIndices = [0, 1, 2]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Mesh"));

    Mesh mesh;
    mesh.extractFromPrim(prim);

    SECTION("Returns non-null vertex pointer") {
        const float* vertices = mesh.getVertices();
        REQUIRE(vertices != nullptr);
    }

    SECTION("Vertex data is correctly formatted") {
        const float* vertices = mesh.getVertices();
        // Check first vertex (x, y, z)
        REQUIRE(vertices[0] == Approx(1.0f));
        REQUIRE(vertices[1] == Approx(2.0f));
        REQUIRE(vertices[2] == Approx(3.0f));
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("Mesh: Get index data", "[usd][mesh]") {
    std::string tempFile = "/tmp/test_index_data.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Mesh \"Mesh\" {\n";
    file << "    point3f[] points = [(0, 0, 0), (1, 0, 0), (0, 1, 0)]\n";
    file << "    int[] faceVertexCounts = [3]\n";
    file << "    int[] faceVertexIndices = [0, 1, 2]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Mesh"));

    Mesh mesh;
    mesh.extractFromPrim(prim);

    SECTION("Returns non-null index pointer") {
        const uint32_t* indices = mesh.getIndices();
        REQUIRE(indices != nullptr);
    }

    SECTION("Index values are correct") {
        const uint32_t* indices = mesh.getIndices();
        REQUIRE(indices[0] == 0);
        REQUIRE(indices[1] == 1);
        REQUIRE(indices[2] == 2);
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("Mesh: Clear mesh data", "[usd][mesh]") {
    std::string tempFile = "/tmp/test_clear.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Mesh \"Mesh\" {\n";
    file << "    point3f[] points = [(0, 0, 0), (1, 0, 0)]\n";
    file << "    int[] faceVertexCounts = [2]\n";
    file << "    int[] faceVertexIndices = [0, 1]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Mesh"));

    Mesh mesh;
    mesh.extractFromPrim(prim);
    REQUIRE(mesh.getVertexCount() > 0);

    SECTION("Clear resets all data") {
        mesh.clear();
        REQUIRE(mesh.getVertexCount() == 0);
        REQUIRE(mesh.getIndexCount() == 0);
        REQUIRE(mesh.getNormalCount() == 0);
        REQUIRE(mesh.getUVCount() == 0);
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("Mesh: Extract from non-mesh prim", "[usd][mesh]") {
    std::string tempFile = "/tmp/test_non_mesh.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"NotAMesh\" {\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/NotAMesh"));

    Mesh mesh;

    SECTION("Fails to extract from Xform prim") {
        REQUIRE_FALSE(mesh.extractFromPrim(prim));
    }

    SECTION("Mesh remains empty after failed extract") {
        mesh.extractFromPrim(prim);
        REQUIRE(mesh.getVertexCount() == 0);
        REQUIRE(mesh.getIndexCount() == 0);
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("Mesh: Mesh with no faces", "[usd][mesh]") {
    std::string tempFile = "/tmp/test_no_faces.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Mesh \"Mesh\" {\n";
    file << "    point3f[] points = [(0, 0, 0), (1, 0, 0), (0, 1, 0)]\n";
    file << "    int[] faceVertexCounts = []\n";
    file << "    int[] faceVertexIndices = []\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Mesh"));

    Mesh mesh;
    mesh.extractFromPrim(prim);

    SECTION("Has vertices but no indices") {
        REQUIRE(mesh.getVertexCount() == 3);
        REQUIRE(mesh.getIndexCount() == 0);
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("Mesh: Get normal and UV data pointers", "[usd][mesh]") {
    std::string tempFile = "/tmp/test_data_pointers.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Mesh \"Mesh\" {\n";
    file << "    point3f[] points = [(0, 0, 0), (1, 0, 0), (0, 1, 0)]\n";
    file << "    int[] faceVertexCounts = [3]\n";
    file << "    int[] faceVertexIndices = [0, 1, 2]\n";
    file << "    normal3f[] normals = [(0, 0, 1), (0, 0, 1), (0, 0, 1)]\n";
    file << "    uniform token normalInterpolation = \"vertex\"\n";
    file << "    float2[] primvars:st = [(0, 0), (1, 0), (0, 1)]\n";
    file << "    uniform token primvars:st interpolation = \"vertex\"\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Mesh"));

    Mesh mesh;
    mesh.extractFromPrim(prim);

    SECTION("Returns valid normal pointer") {
        const float* normals = mesh.getNormals();
        REQUIRE(normals != nullptr);
    }

    SECTION("Returns valid UV pointer") {
        const float* uvs = mesh.getUVs();
        REQUIRE(uvs != nullptr);
    }

    // Cleanup
    std::remove(tempFile.c_str());
}
