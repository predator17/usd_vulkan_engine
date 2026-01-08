#include "bounds.hpp"

namespace usd_ve {

void Bounds::computeFromVertices(const std::vector<glm::vec3>& vertices) {
    if (vertices.empty()) {
        m_min = glm::vec3(0);
        m_max = glm::vec3(0);
        m_valid = false;
        return;
    }

    m_min = m_max = vertices[0];
    m_valid = true;

    for (const auto& v : vertices) {
        m_min = glm::min(m_min, v);
        m_max = glm::max(m_max, v);
    }
}

void Bounds::expand(const glm::vec3& point) {
    if (!m_valid) {
        m_min = m_max = point;
        m_valid = true;
    } else {
        m_min = glm::min(m_min, point);
        m_max = glm::max(m_max, point);
    }
}

void Bounds::expand(const Bounds& other) {
    if (other.isValid()) {
        expand(other.m_min);
        expand(other.m_max);
    }
}

glm::vec3 Bounds::getCenter() const {
    if (!m_valid) return glm::vec3(0);
    return (m_min + m_max) * 0.5f;
}

float Bounds::getRadius() const {
    if (!m_valid) return 0.0f;
    return glm::length(getSize()) * 0.5f;
}

bool Bounds::contains(const glm::vec3& point) const {
    if (!m_valid) return false;
    return point.x >= m_min.x && point.x <= m_max.x &&
           point.y >= m_min.y && point.y <= m_max.y &&
           point.z >= m_min.z && point.z <= m_max.z;
}

bool Bounds::intersects(const Bounds& other) const {
    if (!m_valid || !other.isValid()) return false;
    return m_min.x <= other.m_max.x && m_max.x >= other.m_min.x &&
           m_min.y <= other.m_max.y && m_max.y >= other.m_min.y &&
           m_min.z <= other.m_max.z && m_max.z >= other.m_min.z;
}

} // namespace usd_ve
