#include <catch2/catch_all.hpp>
using Catch::Approx;  // Catch2 v3 requires explicit namespace for Approx
#include <usd/mesh.hpp>
#include <glm/glm.hpp>

// Mesh is a simple container class - it doesn't have USD extraction functionality yet
// This test file verifies the basic container functionality

TEST_CASE("Mesh: Default construction", "[usd][mesh]") {
    Mesh mesh;

    SECTION("Initially has no vertices") {
        REQUIRE(mesh.getVertexCount() == 0);
    }

    SECTION("Initially has no indices") {
        REQUIRE(mesh.getIndexCount() == 0);
    }

    SECTION("Initially is not valid") {
        REQUIRE_FALSE(mesh.isValid());
    }
}

TEST_CASE("Mesh: Set and get vertices", "[usd][mesh]") {
    Mesh mesh;
    std::vector<glm::vec3> vertices = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    };

    mesh.setVertices(vertices);

    SECTION("Vertex count is correct") {
        REQUIRE(mesh.getVertexCount() == 3);
    }

    SECTION("Vertices can be retrieved") {
        const auto& retrieved = mesh.getVertices();
        REQUIRE(retrieved.size() == 3);
        REQUIRE(retrieved[0] == glm::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(retrieved[1] == glm::vec3(1.0f, 0.0f, 0.0f));
        REQUIRE(retrieved[2] == glm::vec3(0.0f, 1.0f, 0.0f));
    }

    SECTION("Mesh is valid after setting vertices") {
        REQUIRE(mesh.isValid());
    }
}

TEST_CASE("Mesh: Set and get indices", "[usd][mesh]") {
    Mesh mesh;
    std::vector<uint32_t> indices = {0, 1, 2};

    mesh.setIndices(indices);

    SECTION("Index count is correct") {
        REQUIRE(mesh.getIndexCount() == 3);
    }

    SECTION("Indices can be retrieved") {
        const auto& retrieved = mesh.getIndices();
        REQUIRE(retrieved.size() == 3);
        REQUIRE(retrieved[0] == 0);
        REQUIRE(retrieved[1] == 1);
        REQUIRE(retrieved[2] == 2);
    }
}

TEST_CASE("Mesh: Set and get normals", "[usd][mesh]") {
    Mesh mesh;
    std::vector<glm::vec3> normals = {
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };

    mesh.setNormals(normals);

    SECTION("Normals can be retrieved") {
        const auto& retrieved = mesh.getNormals();
        REQUIRE(retrieved.size() == 3);
        REQUIRE(retrieved[0] == glm::vec3(0.0f, 0.0f, 1.0f));
    }
}

TEST_CASE("Mesh: Set and get UVs", "[usd][mesh]") {
    Mesh mesh;
    std::vector<glm::vec2> uvs = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.5f, 1.0f)
    };

    mesh.setUVs(uvs);

    SECTION("UVs can be retrieved") {
        const auto& retrieved = mesh.getUVs();
        REQUIRE(retrieved.size() == 3);
        REQUIRE(retrieved[0] == glm::vec2(0.0f, 0.0f));
        REQUIRE(retrieved[1] == glm::vec2(1.0f, 0.0f));
        REQUIRE(retrieved[2] == glm::vec2(0.5f, 1.0f));
    }
}

TEST_CASE("Mesh: Complete mesh data", "[usd][mesh]") {
    Mesh mesh;

    std::vector<glm::vec3> vertices = {
        glm::vec3(-0.5f, -0.5f, 0.0f),
        glm::vec3(0.5f, -0.5f, 0.0f),
        glm::vec3(0.0f, 0.5f, 0.0f)
    };
    std::vector<uint32_t> indices = {0, 1, 2};
    std::vector<glm::vec3> normals = {
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };
    std::vector<glm::vec2> uvs = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.5f, 1.0f)
    };

    mesh.setVertices(vertices);
    mesh.setIndices(indices);
    mesh.setNormals(normals);
    mesh.setUVs(uvs);

    SECTION("All data is set correctly") {
        REQUIRE(mesh.getVertexCount() == 3);
        REQUIRE(mesh.getIndexCount() == 3);
        REQUIRE(mesh.getNormals().size() == 3);
        REQUIRE(mesh.getUVs().size() == 3);
        REQUIRE(mesh.isValid());
    }
}
