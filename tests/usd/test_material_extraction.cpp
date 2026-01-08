#include <catch2/catch.hpp>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/shader.h>
#include <fstream>

using namespace usd_vk;

TEST_CASE("Material extraction: From Material prim", "[usd][material]") {
    std::string tempFile = "/tmp/test_material.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Material \"MyMaterial\" (\n";
    file << "    prepend apiSchemas = [\"MaterialBindingAPI\"]\n";
    file << ") {\n";
    file << "    token outputs:mdl:displacement.connect = </MyMaterial/Displacement.outputs.displacement>\n";
    file << "    token outputs:mdl:surface.connect = </MyMaterial/Surface.outputs.out>\n";
    file << "    token outputs:mdl:volume.connect = </MyMaterial/Volume.outputs.out>\n";
    file << "    def Shader \"Surface\" {\n";
    file << "        token info:implementationSource = \"sourceAsset\"\n";
    file << "        asset info:mdl:sourceAsset = @./.mdl@\n";
    file << "        token info:mdl:sourceAsset:subIdentifier = \"main\"\n";
    file << "        float3 inputs:color = (1, 0, 0)\n";
    file << "        float inputs:roughness = 0.5\n";
    file << "        token outputs:out\n";
    file << "    }\n";
    file << "    def Shader \"Displacement\" {\n";
    file << "        token info:implementationSource = \"sourceAsset\"\n";
    file << "        asset info:mdl:sourceAsset = @./displacement.mdl@\n";
    file << "        token info:mdl:sourceAsset:subIdentifier = \"displace\"\n";
    file << "        float inputs:magnitude = 1\n";
    file << "        token outputs:displacement\n";
    file << "    }\n";
    file << "    def Shader \"Volume\" {\n";
    file << "        token info:implementationSource = \"sourceAsset\"\n";
    file << "        asset info:mdl:sourceAsset = @./volume.mdl@\n";
    file << "        token info:mdl:sourceAsset:subIdentifier = \"volume\"\n";
    file << "        float inputs:density = 1\n";
    file << "        token outputs:out\n";
    file << "    }\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    REQUIRE(stage);

    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/MyMaterial"));
    REQUIRE(prim);

    SECTION("Can access Material prim") {
        pxr::UsdShadeMaterial material(prim);
        REQUIRE(material);
    }

    SECTION("Material has surface output") {
        pxr::UsdShadeMaterial material(prim);
        pxr::UsdShadeOutput surfaceOutput = material.GetSurfaceOutput();
        REQUIRE(surfaceOutput);
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Material extraction: Basic color material", "[usd][material]") {
    std::string tempFile = "/tmp/test_color_material.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Material \"RedMaterial\" {\n";
    file << "    def Shader \"PreviewSurface\" (\n";
    file << "        prepend apiSchemas = [\"MaterialBindingAPI\"]\n";
    file << "    ) {\n";
    file << "        uniform token info:implementationSource = \"typeId\"\n";
    file <<"        uniform token info:id = \"UsdPreviewSurface\"\n";
    file << "        color3f inputs:diffuseColor = (1, 0, 0)\n";
    file << "        float inputs:metallic = 0\n";
    file << "        float inputs:roughness = 0.5\n";
    file << "        token outputs:displacement\n";
    file << "        token outputs:surface\n";
    file << "    }\n";
    file << "    token outputs:mdl:surface.connect = </RedMaterial/PreviewSurface.outputs.surface>\n";
    file << "    token outputs:surface.connect = </RedMaterial/PreviewSurface.outputs.surface>\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath("/RedMaterial"));

    SECTION("Material has preview surface shader") {
        pxr::UsdShadeMaterial material(prim);
        pxr::UsdShadeShader surfaceShader = material.ComputeSurfaceSource();
        REQUIRE(surfaceShader);
    }

    SECTION("Can read diffuse color input") {
        pxr::UsdPrim shaderPrim = stage->GetPrimAtPath(pxr::SdfPath("/RedMaterial/PreviewSurface"));
        pxr::UsdShadeShader shader(shaderPrim);
        pxr::UsdShadeInput colorInput = shader.GetInput(pxr::TfToken("diffuseColor"));
        REQUIRE(colorInput);

        pxr::VtValue value;
        colorInput.Get(&value);
        REQUIRE(value.IsHolding<pxr::GfVec3f>());

        pxr::GfVec3f color = value.Get<pxr::GfVec3f>();
        REQUIRE(color[0] == Approx(1.0f));
        REQUIRE(color[1] == Approx(0.0f));
        REQUIRE(color[2] == Approx(0.0f));
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Material extraction: Metallic roughness workflow", "[usd][material]") {
    std::string tempFile = "/tmp/test_mr_material.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Material \"MetalRoughMat\" {\n";
    file << "    def Shader \"PreviewSurface\" {\n";
    file << "        uniform token info:id = \"UsdPreviewSurface\"\n";
    file << "        color3f inputs:diffuseColor = (0.5, 0.5, 0.5)\n";
    file << "        float inputs:metallic = 1\n";
    file << "        float inputs:roughness = 0.2\n";
    file << "        token outputs:surface\n";
    file << "    }\n";
    file << "    token outputs:surface.connect = </MetalRoughMat/PreviewSurface.outputs.surface>\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim shaderPrim = stage->GetPrimAtPath(pxr::SdfPath("/MetalRoughMat/PreviewSurface"));
    pxr::UsdShadeShader shader(shaderPrim);

    SECTION("Can read metallic value") {
        pxr::UsdShadeInput metallicInput = shader.GetInput(pxr::TfToken("metallic"));
        pxr::VtValue value;
        metallicInput.Get(&value);
        REQUIRE(value.IsHolding<float>());
        REQUIRE(value.Get<float>() == Approx(1.0f));
    }

    SECTION("Can read roughness value") {
        pxr::UsdShadeInput roughnessInput = shader.GetInput(pxr::TfToken("roughness"));
        pxr::VtValue value;
        roughnessInput.Get(&value);
        REQUIRE(value.IsHolding<float>());
        REQUIRE(value.Get<float>() == Approx(0.2f));
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Material extraction: Texture inputs", "[usd][material]") {
    std::string tempFile = "/tmp/test_texture_material.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Material \"TexturedMat\" {\n";
    file << "    def Shader \"PreviewSurface\" {\n";
    file << "        token info:id = \"UsdPreviewSurface\"\n";
    file << "        token inputs:diffuseColor.connect = </TexturedMat/diffuseTexture.outputs:rgb>\n";
    file << "        token outputs:surface\n";
    file << "    }\n";
    file << "    def Shader \"diffuseTexture\" {\n";
    file << "        token info:id = \"UsdUVTexture\"\n";
    file << "        asset inputs:file = @./texture.png@\n";
    file << "        float2 inputs:st.connect = </TexturedMat/stReader.outputs:result>\n";
    file << "        float3f outputs:rgb\n";
    file << "    }\n";
    file << "    def Shader \"stReader\" {\n";
    file << "        token info:id = \"UsdPrimvarReader_float2\"\n";
    file << "        token inputs:varname = \"st\"\n";
    file << "        float2 outputs:result\n";
    file << "    }\n";
    file << "    token outputs:surface.connect = </TexturedMat/PreviewSurface.outputs.surface>\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim texturePrim = stage->GetPrimAtPath(pxr::SdfPath("/TexturedMat/diffuseTexture"));
    pxr::UsdShadeShader textureShader(texturePrim);

    SECTION("Texture shader exists") {
        REQUIRE(textureShader);
    }

    SECTION("Can read texture file path") {
        pxr::UsdShadeInput fileInput = textureShader.GetInput(pxr::TfToken("file"));
        REQUIRE(fileInput);

        pxr::SdfAssetPath assetPath;
        fileInput.Get(&assetPath);
        REQUIRE(assetPath.GetAssetPath() == "./texture.png");
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Material extraction: Clearcoat workflow", "[usd][material]") {
    std::string tempFile = "/tmp/test_clearcoat.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Material \"ClearcoatMat\" {\n";
    file << "    def Shader \"PreviewSurface\" {\n";
    file << "        token info:id = \"UsdPreviewSurface\"\n";
    file << "        color3f inputs:diffuseColor = (0.2, 0.2, 0.8)\n";
    file << "        float inputs:clearcoat = 1\n";
    file << "        float inputs:clearcoatRoughness = 0.1\n";
    file << "        token outputs:surface\n";
    file << "    }\n";
    file << "    token outputs:surface.connect = </ClearcoatMat/PreviewSurface.outputs.surface>\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim shaderPrim = stage->GetPrimAtPath(pxr::SdfPath("/ClearcoatMat/PreviewSurface"));
    pxr::UsdShadeShader shader(shaderPrim);

    SECTION("Can read clearcoat value") {
        pxr::UsdShadeInput clearcoatInput = shader.GetInput(pxr::TfToken("clearcoat"));
        pxr::VtValue value;
        clearcoatInput.Get(&value);
        REQUIRE(value.Get<float>() == Approx(1.0f));
    }

    SECTION("Can read clearcoat roughness") {
        pxr::UsdShadeInput ccRoughnessInput = shader.GetInput(pxr::TfToken("clearcoatRoughness"));
        pxr::VtValue value;
        ccRoughnessInput.Get(&value);
        REQUIRE(value.Get<float>() == Approx(0.1f));
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Material extraction: Emissive material", "[usd][material]") {
    std::string tempFile = "/tmp/test_emissive.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Material \"EmissiveMat\" {\n";
    file << "    def Shader \"PreviewSurface\" {\n";
    file << "        token info:id = \"UsdPreviewSurface\"\n";
    file << "        color3f inputs:diffuseColor = (0, 0, 0)\n";
    file << "        color3f inputs:emissiveColor = (1, 0.5, 0.2)\n";
    file << "        token outputs:surface\n";
    file << "    }\n";
    file << "    token outputs:surface.connect = </EmissiveMat/PreviewSurface.outputs.surface>\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim shaderPrim = stage->GetPrimAtPath(pxr::SdfPath("/EmissiveMat/PreviewSurface"));
    pxr::UsdShadeShader shader(shaderPrim);

    SECTION("Can read emissive color") {
        pxr::UsdShadeInput emissiveInput = shader.GetInput(pxr::TfToken("emissiveColor"));
        pxr::VtValue value;
        emissiveInput.Get(&value);
        REQUIRE(value.IsHolding<pxr::GfVec3f>());

        pxr::GfVec3f color = value.Get<pxr::GfVec3f>();
        REQUIRE(color[0] == Approx(1.0f));
        REQUIRE(color[1] == Approx(0.5f));
        REQUIRE(color[2] == Approx(0.2f));
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Material extraction: Material binding", "[usd][material]") {
    std::string tempFile = "/tmp/test_material_binding.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Material \"MyMaterial\" {\n";
    file << "    token outputs:surface.connect = </MyMaterial/Shader.outputs:out>\n";
    file << "    def Shader \"Shader\" {\n";
    file << "        token info:id = \"UsdPreviewSurface\"\n";
    file << "        color3f inputs:diffuseColor = (1, 0, 0)\n";
    file << "        token outputs:out\n";
    file << "    }\n";
    file << "}\n";
    file << "def Mesh \"Mesh1\" (\n";
    file << "    prepend apiSchemas = [\"MaterialBindingAPI\"]\n";
    file << ") {\n";
    file << "    rel material:binding = </MyMaterial>\n";
    file << "    int[] faceVertexCounts = [3]\n";
    file << "    int[] faceVertexIndices = [0, 1, 2]\n";
    file << "    point3f[] points = [(0, 0, 0), (1, 0, 0), (0, 1, 0)]\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim meshPrim = stage->GetPrimAtPath(pxr::SdfPath("/Mesh1"));

    SECTION("Mesh has material binding") {
        pxr::UsdShadeMaterialBindingAPI bindingAPI(meshPrim);
        pxr::UsdShadeMaterial material = bindingAPI.ComputeBoundMaterial();
        REQUIRE(material);
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Material extraction: Opacity/Transparency", "[usd][material]") {
    std::string tempFile = "/tmp/test_opacity.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Material \"TransparentMat\" {\n";
    file << "    def Shader \"PreviewSurface\" {\n";
    file << "        token info:id = \"UsdPreviewSurface\"\n";
    file << "        color3f inputs:diffuseColor = (1, 0, 0)\n";
    file << "        float inputs:opacity = 0.5\n";
    file << "        token outputs:opacity\n";
    file << "        token outputs:surface\n";
    file << "    }\n";
    file << "    token outputs:surface.connect = </TransparentMat/PreviewSurface.outputs.surface>\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim shaderPrim = stage->GetPrimAtPath(pxr::SdfPath("/TransparentMat/PreviewSurface"));
    pxr::UsdShadeShader shader(shaderPrim);

    SECTION("Can read opacity value") {
        pxr::UsdShadeInput opacityInput = shader.GetInput(pxr::TfToken("opacity"));
        pxr::VtValue value;
        opacityInput.Get(&value);
        REQUIRE(value.Get<float>() == Approx(0.5f));
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Material extraction: IOR and transmission", "[usd][material]") {
    std::string tempFile = "/tmp/test_ior.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Material \"GlassMat\" {\n";
    file << "    def Shader \"PreviewSurface\" {\n";
    file << "        token info:id = \"UsdPreviewSurface\"\n";
    file << "        color3f inputs:diffuseColor = (1, 1, 1)\n";
    file << "        float inputs:ior = 1.5\n";
    file << "        float inputs:transmission = 1\n";
    file << "        token outputs:surface\n";
    file << "    }\n";
    file << "    token outputs:surface.connect = </GlassMat/PreviewSurface.outputs.surface>\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    pxr::UsdPrim shaderPrim = stage->GetPrimAtPath(pxr::SdfPath("/GlassMat/PreviewSurface"));
    pxr::UsdShadeShader shader(shaderPrim);

    SECTION("Can read IOR value") {
        pxr::UsdShadeInput iorInput = shader.GetInput(pxr::TfToken("ior"));
        pxr::VtValue value;
        iorInput.Get(&value);
        REQUIRE(value.Get<float>() == Approx(1.5f));
    }

    SECTION("Can read transmission value") {
        pxr::UsdShadeInput transmissionInput = shader.GetInput(pxr::TfToken("transmission"));
        pxr::VtValue value;
        transmissionInput.Get(&value);
        REQUIRE(value.Get<float>() == Approx(1.0f));
    }

    std::remove(tempFile.c_str());
}

TEST_CASE("Material extraction: Multiple materials in scene", "[usd][material]") {
    std::string tempFile = "/tmp/test_multiple_materials.usda";
    std::ofstream file(tempFile);
    file << "#usda 1.0\n";
    file << "def Material \"Material1\" {\n";
    file << "    def Shader \"Shader\" {\n";
    file << "        token info:id = \"UsdPreviewSurface\"\n";
    file << "        color3f inputs:diffuseColor = (1, 0, 0)\n";
    file << "        token outputs:out\n";
    file << "    }\n";
    file << "    token outputs:surface.connect = </Material1/Shader.outputs:out>\n";
    file << "}\n";
    file << "def Material \"Material2\" {\n";
    file << "    def Shader \"Shader\" {\n";
    file << "        token info:id = \"UsdPreviewSurface\"\n";
    file << "        color3f inputs:diffuseColor = (0, 1, 0)\n";
    file << "        token outputs:out\n";
    file << "    }\n";
    file << "    token outputs:surface.connect = </Material2/Shader.outputs:out>\n";
    file << "}\n";
    file.close();

    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(tempFile);
    REQUIRE(stage);

    pxr::UsdPrim mat1Prim = stage->GetPrimAtPath(pxr::SdfPath("/Material1"));
    pxr::UsdPrim mat2Prim = stage->GetPrimAtPath(pxr::SdfPath("/Material2"));

    SECTION("Both materials can be accessed") {
        pxr::UsdShadeMaterial mat1(mat1Prim);
        pxr::UsdShadeMaterial mat2(mat2Prim);
        REQUIRE(mat1);
        REQUIRE(mat2);
    }

    std::remove(tempFile.c_str());
}
