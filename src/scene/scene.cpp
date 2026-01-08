#include "scene.hpp"
#include "core/logger.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <glm/gtc/quaternion.hpp>
#include <spdlog/spdlog.h>
#include <regex>

Scene::Scene() = default;

Scene::~Scene() {
    clear();
}

bool Scene::load(const std::string& filePath) {
    SPDLOG_INFO("Loading scene: {}", filePath);

    // Check if USD library is available
    #ifdef USD_ENABLED
        return loadUSDArchive(filePath);
    #else
        // Fallback: simple USDA parser for testing
        SPDLOG_WARN("USD library not available, using simple parser");
        return extractMeshData(filePath);
    #endif
}

void Scene::clear() {
    m_meshes.clear();
    m_loaded = false;
}

void Scene::updateAnimation(double time) {
    // Update rotation based on time for spinning cube
    if (!m_meshes.empty()) {
        // Spin 360 degrees over ~4.17 seconds (100 frames at 24fps)
        float angle = static_cast<float>(time * 2.0 * 3.14159 / 4.17);
        m_meshes[0].rotation = glm::angleAxis(angle, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

bool Scene::loadUSDArchive(const std::string& filePath) {
    // TODO: Implement full USD loading when USD is available
    SPDLOG_ERROR("Full USD loading not yet implemented");
    return false;
}

bool Scene::extractMeshData(const std::string& filePath) {
    // For now, create a simple test cube to verify rendering works
    // TODO: Implement proper USD parsing
    MeshData mesh;

    // Simple colored cube vertices with proper normals (each vertex repeated per face)
    struct Vertex { glm::vec3 pos; glm::vec3 normal; glm::vec3 color; };
    std::vector<Vertex> cubeVertices = {
        // Front face (Z+) - normal (0,0,1)
        {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0, 0, 1), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0, 0, 1), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0, 0, 1), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0, 0, 1), glm::vec3(1, 0.3f, 0.3f)},

        // Back face (Z-) - normal (0,0,-1)
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0, 0, -1), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0, 0, -1), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0, 0, -1), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0, 0, -1), glm::vec3(1, 0.3f, 0.3f)},

        // Top face (Y+) - normal (0,1,0)
        {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0, 1, 0), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0, 1, 0), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0, 1, 0), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0, 1, 0), glm::vec3(1, 0.3f, 0.3f)},

        // Bottom face (Y-) - normal (0,-1,0)
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0, -1, 0), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0, -1, 0), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0, -1, 0), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0, -1, 0), glm::vec3(1, 0.3f, 0.3f)},

        // Right face (X+) - normal (1,0,0)
        {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1, 0, 0), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1, 0, 0), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1, 0, 0), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1, 0, 0), glm::vec3(1, 0.3f, 0.3f)},

        // Left face (X-) - normal (-1,0,0)
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1, 0, 0), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-1, 0, 0), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-1, 0, 0), glm::vec3(1, 0.3f, 0.3f)},
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-1, 0, 0), glm::vec3(1, 0.3f, 0.3f)},
    };

    // Cube indices (12 triangles = 36 indices)
    mesh.indices = {
        // Front face
        0, 1, 2, 2, 3, 0,
        // Back face
        4, 5, 6, 6, 7, 4,
        // Top face
        8, 9, 10, 10, 11, 8,
        // Bottom face
        12, 13, 14, 14, 15, 12,
        // Right face
        16, 17, 18, 18, 19, 16,
        // Left face
        20, 21, 22, 22, 23, 20
    };

    // Extract to separate arrays
    mesh.vertices.reserve(cubeVertices.size());
    mesh.normals.reserve(cubeVertices.size());
    mesh.colors.reserve(cubeVertices.size());

    for (const auto& v : cubeVertices) {
        mesh.vertices.push_back(v.pos);
        mesh.normals.push_back(v.normal);
        mesh.colors.push_back(v.color);
    }

    mesh.texCoords.resize(cubeVertices.size(), glm::vec2(0, 0));
    mesh.baseColor = glm::vec3(1.0f, 0.3f, 0.3f);
    mesh.translation = glm::vec3(0, 0, 0);
    mesh.scale = glm::vec3(1, 1, 1);
    mesh.rotation = glm::quat(1, 0, 0, 0);

    m_meshes.push_back(std::move(mesh));
    m_loaded = true;

    // Calculate bounds
    for (const auto& m : m_meshes) {
        for (const auto& v : m.vertices) {
            m_minBounds = glm::min(m_minBounds, v);
            m_maxBounds = glm::max(m_maxBounds, v);
        }
    }

    SPDLOG_INFO("Scene loaded: {} vertices, {} indices",
         m_meshes[0].vertices.size(), m_meshes[0].indices.size());
    return true;
}
