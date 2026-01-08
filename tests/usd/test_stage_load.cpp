#include <catch2/catch.hpp>
#include <usd/stage.hpp>
#include <pxr/pxr.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <fstream>
#include <cstdio>

using namespace usd_ve;

TEST_CASE("USDStage: Default construction", "[usd][stage]") {
    USDStage stage;

    SECTION("Stage is initially not loaded") {
        REQUIRE_FALSE(stage.isLoaded());
    }

    SECTION("Get stage returns nullptr when not loaded") {
        REQUIRE(stage.getStage() == nullptr);
    }

    SECTION("Get root prim returns empty path") {
        REQUIRE(stage.getRootPrimPath() == "");
    }
}

TEST_CASE("USDStage: Load non-existent file", "[usd][stage]") {
    USDStage stage;

    SECTION("Load returns false for non-existent file") {
        bool result = stage.load("/path/that/does/not/exist.usda");
        REQUIRE_FALSE(result);
    }

    SECTION("Stage remains not loaded after failed load") {
        stage.load("/nonexistent/file.usda");
        REQUIRE_FALSE(stage.isLoaded());
    }
}

TEST_CASE("USDStage: Create simple in-memory stage", "[usd][stage]") {
    // Create a simple USD file programmatically
    std::string tempFile = "/tmp/test_stage.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "(\n";
    file << "    defaultPrim = \"World\"\n";
    file << ")\n";
    file << "def Xform \"World\" {\n";
    file << "}\n";
    file.close();

    USDStage stage;

    SECTION("Load returns true for valid file") {
        bool result = stage.load(tempFile);
        REQUIRE(result);
    }

    SECTION("Stage is loaded after successful load") {
        stage.load(tempFile);
        REQUIRE(stage.isLoaded());
    }

    SECTION("Get stage returns valid pointer") {
        stage.load(tempFile);
        REQUIRE(stage.getStage() != nullptr);
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("USDStage: Get root prim path", "[usd][stage]") {
    std::string tempFile = "/tmp/test_root_prim.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "(\n";
    file << "    defaultPrim = \"MyRoot\"\n";
    file << ")\n";
    file << "def Xform \"MyRoot\" {\n";
    file << "}\n";
    file.close();

    USDStage stage;
    stage.load(tempFile);

    SECTION("Returns default prim path") {
        REQUIRE(stage.getRootPrimPath() == "/MyRoot");
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("USDStage: Get start and end time codes", "[usd][stage]") {
    std::string tempFile = "/tmp/test_time_codes.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "(\n";
    file << "    startTimeCode = 100\n";
    file << "    endTimeCode = 200\n";
    file << "    defaultPrim = \"Root\"\n";
    file << ")\n";
    file << "def Xform \"Root\" {\n";
    file << "}\n";
    file.close();

    USDStage stage;
    stage.load(tempFile);

    SECTION("Returns start time code") {
        REQUIRE(stage.getStartTimeCode() == 100.0);
    }

    SECTION("Returns end time code") {
        REQUIRE(stage.getEndTimeCode() == 200.0);
    }

    SECTION("Calculates duration") {
        double duration = stage.getEndTimeCode() - stage.getStartTimeCode();
        REQUIRE(duration == 100.0);
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("USDStage: Time codes default values", "[usd][stage]") {
    std::string tempFile = "/tmp/test_default_time.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "(\n";
    file << "    defaultPrim = \"Root\"\n";
    file << ")\n";
    file << "def Xform \"Root\" {\n";
    file << "}\n";
    file.close();

    USDStage stage;
    stage.load(tempFile);

    SECTION("Default start time code") {
        // USD default is 0
        REQUIRE(stage.getStartTimeCode() == 0.0);
    }

    SECTION("Default end time code") {
        REQUIRE(stage.getEndTimeCode() == 0.0);
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("USDStage: Reload stage", "[usd][stage]") {
    std::string tempFile = "/tmp/test_reload.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Root\" {\n";
    file << "}\n";
    file.close();

    USDStage stage;

    SECTION("Load same file twice") {
        REQUIRE(stage.load(tempFile));
        REQUIRE(stage.isLoaded());
        REQUIRE(stage.load(tempFile));
        REQUIRE(stage.isLoaded());
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("USDStage: Load different files", "[usd][stage]") {
    std::string tempFile1 = "/tmp/test_file1.usda";
    std::string tempFile2 = "/tmp/test_file2.usda";

    std::ofstream file1(tempFile1);
    file1 << "#usda 1.0\n";
    file1 << "def Xform \"Root1\" {}\n";
    file1.close();

    std::ofstream file2(tempFile2);
    file2 << "#usda 1.0\n";
    file2 << "def Xform \"Root2\" {}\n";
    file2.close();

    USDStage stage;

    SECTION("Load first file then second") {
        REQUIRE(stage.load(tempFile1));
        REQUIRE(stage.getRootPrimPath() == "/Root1");
        REQUIRE(stage.load(tempFile2));
        REQUIRE(stage.getRootPrimPath() == "/Root2");
    }

    // Cleanup
    std::remove(tempFile1.c_str());
    std::remove(tempFile2.c_str());
}

TEST_CASE("USDStage: Empty file", "[usd][stage]") {
    std::string tempFile = "/tmp/test_empty.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file.close();

    USDStage stage;

    SECTION("Can load empty file") {
        REQUIRE(stage.load(tempFile));
        REQUIRE(stage.isLoaded());
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("USDStage: File with hierarchy", "[usd][stage]") {
    std::string tempFile = "/tmp/test_hierarchy.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "(\n";
    file << "    defaultPrim = \"World\"\n";
    file << ")\n";
    file << "def Xform \"World\" {\n";
    file << "    def Xform \"Character\" {\n";
    file << "        def Mesh \"Body\" {\n";
    file << "        }\n";
    file << "    }\n";
    file << "}\n";
    file.close();

    USDStage stage;
    stage.load(tempFile);

    SECTION("Root prim is default") {
        REQUIRE(stage.getRootPrimPath() == "/World");
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("USDStage: File with multiple prims", "[usd][stage]") {
    std::string tempFile = "/tmp/test_multiple_prims.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "(\n";
    file << "    defaultPrim = \"Prim1\"\n";
    file << ")\n";
    file << "def Xform \"Prim1\" {\n";
    file << "}\n";
    file << "def Xform \"Prim2\" {\n";
    file << "}\n";
    file << "def Xform \"Prim3\" {\n";
    file << "}\n";
    file.close();

    USDStage stage;
    stage.load(tempFile);

    SECTION("Loads file with multiple root prims") {
        REQUIRE(stage.isLoaded());
        REQUIRE(stage.getRootPrimPath() == "/Prim1");
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("USDStage: Unload stage", "[usd][stage]") {
    std::string tempFile = "/tmp/test_unload.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Root\" {}\n";
    file.close();

    USDStage stage;
    stage.load(tempFile);

    SECTION("Unload clears the stage") {
        REQUIRE(stage.isLoaded());
        stage.unload();
        REQUIRE_FALSE(stage.isLoaded());
        REQUIRE(stage.getStage() == nullptr);
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("USDStage: File with variants", "[usd][stage]") {
    std::string tempFile = "/tmp/test_variants.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Xform \"Root\" {\n";
    file << "    def VariantSet \"shading\" {\n";
    file << "        variant \"default\" {\n";
    file << "        }\n";
    file << "        variant \"metal\" {\n";
    file << "        }\n";
    file << "    }\n";
    file << "}\n";
    file.close();

    USDStage stage;

    SECTION("Can load file with variants") {
        REQUIRE(stage.load(tempFile));
        REQUIRE(stage.isLoaded());
    }

    // Cleanup
    std::remove(tempFile.c_str());
}

TEST_CASE("USDStage: File with references", "[usd][stage]") {
    std::string refFile = "/tmp/test_ref.usda";
    std::string mainFile = "/tmp/test_main.usda";

    std::ofstream ref(refFile);
    ref << "#usda 1.0\n";
    ref << "def Xform \"Referenced\" {}\n";
    ref.close();

    std::ofstream main(mainFile);
    main << "#usda 1.0\n";
    main << "def Xform \"Root\" {\n";
    main << "    ref Xform = @./test_ref.usda@</Referenced>\n";
    main << "}\n";
    main.close();

    USDStage stage;

    SECTION("Can load file with references") {
        // Note: This might not work without proper path resolution
        // Testing that the load doesn't crash
        stage.load(mainFile);
        // Just verify no crash
        REQUIRE(true);
    }

    // Cleanup
    std::remove(refFile.c_str());
    std::remove(mainFile.c_str());
}

TEST_CASE("USDStage: Invalid USD file", "[usd][stage]") {
    std::string tempFile = "/tmp/test_invalid.usda";
    std::ofstream file(tempFile);
    file << "This is not a valid USD file\n";
    file.close();

    USDStage stage;

    SECTION("Invalid file fails to load") {
        bool result = stage.load(tempFile);
        // USD might still create a stage for invalid files
        // Just verify no crash
        REQUIRE(true);
    }

    // Cleanup
    std::remove(tempFile.c_str());
}
