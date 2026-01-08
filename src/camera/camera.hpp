#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    Camera() = default;

    void setPosition(const glm::vec3& pos) { m_position = pos; }
    void setTarget(const glm::vec3& target) { m_target = target; }
    void setUp(const glm::vec3& up) { m_up = up; }

    glm::vec3 getPosition() const { return m_position; }
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspect, float fov = 45.0f) const;

private:
    glm::vec3 m_position{0.0f, 0.0f, 5.0f};
    glm::vec3 m_target{0.0f, 0.0f, 0.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};
};
