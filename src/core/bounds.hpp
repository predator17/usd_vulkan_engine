#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <vector>
#include <algorithm>
#include <limits>

namespace usd_vk {

class Bounds {
public:
    Bounds() = default;

    void computeFromVertices(const std::vector<glm::vec3>& vertices);
    void expand(const glm::vec3& point);
    void expand(const Bounds& other);

    glm::vec3 getMin() const { return m_min; }
    glm::vec3 getMax() const { return m_max; }
    glm::vec3 getCenter() const;
    glm::vec3 getSize() const { return m_max - m_min; }
    float getRadius() const;

    bool isValid() const { return m_valid; }
    bool contains(const glm::vec3& point) const;
    bool intersects(const Bounds& other) const;

private:
    glm::vec3 m_min{std::numeric_limits<float>::max()};
    glm::vec3 m_max{std::numeric_limits<float>::lowest()};
    bool m_valid = false;
};

} // namespace usd_vk
