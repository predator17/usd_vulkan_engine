#pragma once

#include <glm/glm.hpp>

/**
 * @brief Camera with orbit controls for viewport interaction
 *
 * Uses spherical coordinates (radius, azimuth, elevation) for smooth
 * orbit/rotate around a target point.
 */
class Camera {
public:
    Camera() = default;

    // Position control
    void setPosition(const glm::vec3& pos);
    void setTarget(const glm::vec3& target) { m_target = target; updatePosition(); }
    void setUp(const glm::vec3& up) { m_up = up; }

    // Orbit control parameters
    void setRadius(float radius) { m_radius = glm::max(0.1f, radius); updatePosition(); }
    void setAzimuth(float azimuth) { m_azimuth = azimuth; updatePosition(); }
    void setElevation(float elevation);

    // Getters
    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getTarget() const { return m_target; }
    glm::vec3 getUp() const { return m_up; }
    float getRadius() const { return m_radius; }
    float getAzimuth() const { return m_azimuth; }
    float getElevation() const { return m_elevation; }

    // Matrix generation
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspect, float fov = 45.0f) const;

    // Orbit controls (called from mouse input)
    void orbit(float deltaAzimuth, float deltaElevation);
    void zoom(float delta);
    void pan(float deltaX, float deltaY);

private:
    void updatePosition();
    glm::vec3 forward() const;
    glm::vec3 right() const;

    // Spherical coordinates
    float m_radius = 3.5f;
    float m_azimuth = glm::radians(45.0f);   // Horizontal angle
    float m_elevation = glm::radians(30.0f); // Vertical angle

    glm::vec3 m_position{0.0f, 0.0f, 5.0f};
    glm::vec3 m_target{0.0f, 0.0f, 0.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};
};
