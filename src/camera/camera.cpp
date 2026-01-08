#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_target, m_up);
}

glm::mat4 Camera::getProjectionMatrix(float aspect, float fov) const {
    return glm::perspective(glm::radians(fov), aspect, 0.1f, 1000.0f);
}

void Camera::setPosition(const glm::vec3& pos) {
    m_position = pos;
    // Update spherical coordinates from position
    glm::vec3 offset = m_position - m_target;
    m_radius = glm::length(offset);
    if (m_radius > 0.001f) {
        m_azimuth = std::atan2(offset.x, offset.z);
        m_elevation = std::asin(glm::clamp(offset.y / m_radius, -1.0f, 1.0f));
    }
}

void Camera::setElevation(float elevation) {
    // Clamp elevation to avoid gimbal lock at poles
    m_elevation = glm::clamp(elevation,
        -glm::half_pi<float>() + 0.01f,
        glm::half_pi<float>() - 0.01f);
    updatePosition();
}

void Camera::orbit(float deltaAzimuth, float deltaElevation) {
    m_azimuth += deltaAzimuth;
    setElevation(m_elevation + deltaElevation);
    updatePosition();
}

void Camera::zoom(float delta) {
    // Zoom by adjusting radius
    setRadius(m_radius * (1.0f - delta));
}

void Camera::pan(float deltaX, float deltaY) {
    // Pan in screen space (relative to camera orientation)
    glm::vec3 camRight = right();
    glm::vec3 camUp = m_up;  // Use global up for panning

    float panScale = m_radius * 0.002f;
    m_target += camRight * (-deltaX * panScale);
    m_target += camUp * (deltaY * panScale);
    updatePosition();
}

void Camera::updatePosition() {
    // Convert spherical coordinates to Cartesian
    m_position.x = m_target.x + m_radius * std::sin(m_azimuth) * std::cos(m_elevation);
    m_position.y = m_target.y + m_radius * std::sin(m_elevation);
    m_position.z = m_target.z + m_radius * std::cos(m_azimuth) * std::cos(m_elevation);
}

glm::vec3 Camera::forward() const {
    return glm::normalize(m_target - m_position);
}

glm::vec3 Camera::right() const {
    return glm::normalize(glm::cross(forward(), m_up));
}
