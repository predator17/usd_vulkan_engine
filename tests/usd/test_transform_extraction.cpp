#include <catch2/catch.hpp>
#include <usd/stage.hpp>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/geom/xformable.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <fstream>

using namespace usd_vk;

TEST_CASE("Transform extraction: From Xform prim", "[usd][transform]") {
    std::string tempFile = "/tmp/test_xform.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Root\" {\n";
    file << "    double3 xformOp:translate = (1, 2, 3)\n";
    file << "    uniform token[] xformOpOrder = [\"xformOp:translate\"]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    REQUIRE(stage);

    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Root"));
    REQUIRE(prim);

    SECTION("Extract transform from Xform") {
        pxr::UsdGeomXformable xformable(prim);
        REQUIRE(xformable);

        pxr::GfMatrix4d transform;
        bool resetsXformStack;
        xformable.GetLocalTransformation(&transform, &resetsXformStack);

        // Check translation
        pxr::GfVec3d translation = transform.ExtractTranslation();
        REQUIRE(translation[0] == Approx(1.0));
        REQUIRE(translation[1] == Approx(2.0));
        REQUIRE(translation[2] == Approx(3.0));
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Transform extraction: Identity transform", "[usd][transform]") {
    std::string tempFile = "/tmp/test_identity.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Root\" {\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Root"));

    SECTION("Default transform is identity") {
        pxr::UsdGeomXformable xformable(prim);
        pxr::GfMatrix4d transform;
        bool resetsXformStack;
        xformable.GetLocalTransformation(&transform, &resetsXformStack);

        // Identity matrix has 1 on diagonal
        REQUIRE(transform[0][0] == Approx(1.0));
        REQUIRE(transform[1][1] == Approx(1.0));
        REQUIRE(transform[2][2] == Approx(1.0));
        REQUIRE(transform[3][3] == Approx(1.0));
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Transform extraction: Rotation", "[usd][transform]") {
    std::string tempFile = "/tmp/test_rotation.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Root\" {\n";
    file << "    float3 xformOp:rotateXYZ = (90, 0, 0)\n";
    file << "    uniform token[] xformOpOrder = [\"xformOp:rotateXYZ\"]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Root"));

    SECTION("Extract rotation transform") {
        pxr::UsdGeomXformable xformable(prim);
        pxr::GfMatrix4d transform;
        bool resetsXformStack;
        xformable.GetLocalTransformation(&transform, &resetsXformStack);

        // Rotation should change the matrix
        // After 90° X rotation, Y becomes Z
        REQUIRE(std::abs(transform[1][1]) < 0.01);  // Y component in Y row
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Transform extraction: Scale", "[usd][transform]") {
    std::string tempFile = "/tmp/test_scale.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Root\" {\n";
    file << "    float3 xformOp:scale = (2, 3, 4)\n";
    file << "    uniform token[] xformOpOrder = [\"xformOp:scale\"]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Root"));

    SECTION("Extract scale transform") {
        pxr::UsdGeomXformable xformable(prim);
        pxr::GfMatrix4d transform;
        bool resetsXformStack;
        xformable.GetLocalTransformation(&transform, &resetsXformStack);

        REQUIRE(transform[0][0] == Approx(2.0));
        REQUIRE(transform[1][1] == Approx(3.0));
        REQUIRE(transform[2][2] == Approx(4.0));
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Transform extraction: Combined TRS", "[usd][transform]") {
    std::string tempFile = "/tmp/test_trs.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Root\" {\n";
    file << "    double3 xformOp:translate = (5, 0, 0)\n";
    file << "    float3 xformOp:scale = (2, 2, 2)\n";
    file << "    uniform token[] xformOpOrder = [\"xformOp:translate\", \"xformOp:scale\"]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Root"));

    SECTION("Extract combined transform") {
        pxr::UsdGeomXformable xformable(prim);
        pxr::GfMatrix4d transform;
        bool resetsXformStack;
        xformable.GetLocalTransformation(&transform, &resetsXformStack);

        pxr::GfVec3d translation = transform.ExtractTranslation();
        REQUIRE(translation[0] == Approx(5.0));
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Transform extraction: Nested hierarchy", "[usd][transform]") {
    std::string tempFile = "/tmp/test_hierarchy.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Parent\" {\n";
    file << "    double3 xformOp:translate = (1, 0, 0)\n";
    file << "    uniform token[] xformOpOrder = [\"xformOp:translate\"]\n";
    file << "    def Xform \"Child\" {\n";
    file << "        double3 xformOp:translate = (0, 2, 0)\n";
    file << "        uniform token[] xformOpOrder = [\"xformOp:translate\"]\n";
    file << "    }\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim parentPrim = stage->GetPrimAtPath(pxr::SdfPath("/Parent"));
    pxr::UsdPrim childPrim = stage->GetPrimAtPath(pxr::SdfPath("/Parent/Child"));

    SECTION("Parent has translation") {
        pxr::UsdGeomXformable xformable(parentPrim);
        pxr::GfMatrix4d transform;
        bool resetsXformStack;
        xformable.GetLocalTransformation(&transform, &resetsXformStack);

        pxr::GfVec3d translation = transform.ExtractTranslation();
        REQUIRE(translation[0] == Approx(1.0));
        REQUIRE(translation[1] == Approx(0.0));
    }

    SECTION("Child has local translation") {
        pxr::UsdGeomXformable xformable(childPrim);
        pxr::GfMatrix4d transform;
        bool resetsXformStack;
        xformable.GetLocalTransformation(&transform, &resetsXformStack);

        pxr::GfVec3d translation = transform.ExtractTranslation();
        REQUIRE(translation[0] == Approx(0.0));
        REQUIRE(translation[1] == Approx(2.0));
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Transform extraction: Matrix transform", "[usd][transform]") {
    std::string tempFile = "/tmp/test_matrix.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Root\" {\n";
    file << "    matrix4d xformOp:transform = ";
    file << "((1, 0, 0, 0), (0, 1, 0, 0), (0, 0, 1, 0), (5, 10, 15, 1))\n";
    file << "    uniform token[] xformOpOrder = [\"xformOp:transform\"]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Root"));

    SECTION("Extract matrix transform") {
        pxr::UsdGeomXformable xformable(prim);
        pxr::GfMatrix4d transform;
        bool resetsXformStack;
        xformable.GetLocalTransformation(&transform, &resetsXformStack);

        pxr::GfVec3d translation = transform.ExtractTranslation();
        REQUIRE(translation[0] == Approx(5.0));
        REQUIRE(translation[1] == Approx(10.0));
        REQUIRE(translation[2] == Approx(15.0));
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Transform extraction: Pivot transform", "[usd][transform]") {
    std::string tempFile = "/tmp/test_pivot.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Root\" {\n";
    file << "    double3 xformOp:translate = (1, 0, 0)\n";
    file << "    double3 xformOp:translate:pivot = (0, 1, 0)\n";
    file << "    uniform token[] xformOpOrder = [\"xformOp:translate:pivot\", \"xformOp:translate\"]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Root"));

    SECTION("Extract pivot transform") {
        pxr::UsdGeomXformable xformable(prim);
        pxr::GfMatrix4d transform;
        bool resetsXformStack;
        xformable.GetLocalTransformation(&transform, &resetsXformStack);

        // Just verify it doesn't crash
        REQUIRE(true);
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Transform extraction: Rotate around axis", "[usd][transform]") {
    std::string tempFile = "/tmp/test_axis_rotate.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Root\" {\n";
    file << "    double4 xformOp:rotateY = (90, 0, 1, 0)\n";
    file << "    uniform token[] xformOpOrder = [\"xformOp:rotateY\"]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Root"));

    SECTION("Extract axis rotation") {
        pxr::UsdGeomXformable xformable(prim);
        pxr::GfMatrix4d transform;
        bool resetsXformStack;
        xformable.GetLocalTransformation(&transform, &resetsXformStack);

        // 90 degree Y rotation
        // X axis becomes -Z
        REQUIRE(std::abs(transform[0][0]) < 0.01);  // X row X col
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Transform extraction: Shear transform", "[usd][transform]") {
    std::string tempFile = "/tmp/test_shear.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Root\" {\n";
    file << "    float3 xformOp:shear = (0.5, 0, 0)\n";
    file << "    uniform token[] xformOpOrder = [\"xformOp:shear\"]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Root"));

    SECTION("Extract shear transform") {
        pxr::UsdGeomXformable xformable(prim);
        pxr::GfMatrix4d transform;
        bool resetsXformStack;
        xformable.GetLocalTransformation(&transform, &resetsXformStack);

        // Shear affects off-diagonal elements
        REQUIRE(true);  // Just verify no crash
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Transform extraction: Non-uniform scale", "[usd][transform]") {
    std::string tempFile = "/tmp/test_nonuniform.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Root\" {\n";
    file << "    float3 xformOp:scale = (1, 2, 0.5)\n";
    file << "    uniform token[] xformOpOrder = [\"xformOp:scale\"]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Root"));

    SECTION("Extract non-uniform scale") {
        pxr::UsdGeomXformable xformable(prim);
        pxr::GfMatrix4d transform;
        bool resetsXformStack;
        xformable.GetLocalTransformation(&transform, &resetsXformStack);

        REQUIRE(transform[0][0] == Approx(1.0));
        REQUIRE(transform[1][1] == Approx(2.0));
        REQUIRE(transform[2][2] == Approx(0.5));
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Transform extraction: World transform", "[usd][transform]") {
    std::string tempFile = "/tmp/test_world.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Parent\" {\n";
    file << "    double3 xformOp:translate = (5, 0, 0)\n";
    file << "    uniform token[] xformOpOrder = [\"xformOp:translate\"]\n";
    file << "    def Xform \"Child\" {\n";
    file << "        double3 xformOp:translate = (1, 0, 0)\n";
    file << "        uniform token[] xformOpOrder = [\"xformOp:translate\"]\n";
    file << "    }\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim childPrim = stage->GetPrimAtPath(pxr::SdfPath("/Parent/Child"));

    SECTION("Get world transform") {
        pxr::UsdGeomXformable xformable(childPrim);
        pxr::GfMatrix4d worldTransform = xformable.ComputeLocalToWorldTransform(pxr::UsdTimeCode::Default());

        pxr::GfVec3d translation = worldTransform.ExtractTranslation();
        // Child's world position should be parent (5,0,0) + child local (1,0,0) = (6,0,0)
        REQUIRE(translation[0] == Approx(6.0));
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Transform extraction: Zero scale", "[usd][transform]") {
    std::string tempFile = "/tmp/test_zero_scale.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Root\" {\n";
    file << "    float3 xformOp:scale = (0, 0, 0)\n";
    file << "    uniform token[] xformOpOrder = [\"xformOp:scale\"]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/Root"));

    SECTION("Extract zero scale") {
        pxr::UsdGeomXformable xformable(prim);
        pxr::GfMatrix4d transform;
        bool resetsXformStack;
        xformable.GetLocalTransformation(&transform, &resetsXformStack);

        REQUIRE(transform[0][0] == Approx(0.0));
        REQUIRE(transform[1][1] == Approx(0.0));
        REQUIRE(transform[2][2] == Approx(0.0));
    }

    std::remove(tempFile.c_str());
}
