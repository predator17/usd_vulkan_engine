#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class VulkanContext;

/**
 * @brief Mesh data for rendering
 */
struct MeshData {
    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> colors;

    // Transforms
    glm::vec3 translation = glm::vec3(0.0f);
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    // Material
    glm::vec3 baseColor = glm::vec3(1.0f, 1.0f, 1.0f);
    float metallic = 0.0f;
    float roughness = 0.5f;
};

/**
 * @brief Scene class for managing USD loaded data
 *
 * Loads and stores mesh data from USD files.
 * Provides access to meshes for rendering.
 */
class Scene {
public:
    Scene();
    ~Scene();

    // Non-copyable
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    /**
     * @brief Load a USD scene from file
     * @param filePath Path to .usda/.usdc/.usd file
     * @return true if successful
     */
    bool load(const std::string& filePath);

    /**
     * @brief Get all mesh data
     */
    const std::vector<MeshData>& getMeshes() const { return m_meshes; }

    /**
     * @brief Get mesh count
     */
    size_t getMeshCount() const { return m_meshes.size(); }

    /**
     * @brief Clear all scene data
     */
    void clear();

    /**
     * @brief Update transforms for animation
     * @param time Current time in seconds
     */
    void updateAnimation(double time);

    /**
     * @brief Check if scene is loaded
     */
    bool isLoaded() const { return m_loaded; }

    /**
     * @brief Get scene bounds
     */
    const glm::vec3& getMinBounds() const { return m_minBounds; }
    const glm::vec3& getMaxBounds() const { return m_maxBounds; }

    /**
     * @brief Get animation time range
     */
    double getStartTimeCode() const { return m_startTimeCode; }
    double getEndTimeCode() const { return m_endTimeCode; }

private:
    bool loadUSDArchive(const std::string& filePath);
    bool extractMeshData(const std::string& usdaContent);

    std::vector<MeshData> m_meshes;
    bool m_loaded = false;

    // Bounds
    glm::vec3 m_minBounds = glm::vec3(0.0f);
    glm::vec3 m_maxBounds = glm::vec3(1.0f);

    // Animation
    double m_startTimeCode = 0.0;
    double m_endTimeCode = 100.0 / 24.0;  // 100 frames at 24 fps
};
