#pragma once

#include <vector>
#include <glm/glm.hpp>

class Mesh {
public:
    Mesh() = default;

    const std::vector<glm::vec3>& getVertices() const { return m_vertices; }
    const std::vector<uint32_t>& getIndices() const { return m_indices; }
    const std::vector<glm::vec3>& getNormals() const { return m_normals; }
    const std::vector<glm::vec2>& getUVs() const { return m_uvs; }

    void setVertices(const std::vector<glm::vec3>& v) { m_vertices = v; }
    void setIndices(const std::vector<uint32_t>& i) { m_indices = i; }
    void setNormals(const std::vector<glm::vec3>& n) { m_normals = n; }
    void setUVs(const std::vector<glm::vec2>& u) { m_uvs = u; }

    size_t getVertexCount() const { return m_vertices.size(); }
    size_t getIndexCount() const { return m_indices.size(); }
    bool isValid() const { return !m_vertices.empty(); }

private:
    std::vector<glm::vec3> m_vertices;
    std::vector<uint32_t> m_indices;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_uvs;
};
