#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_target, m_up);
}

glm::mat4 Camera::getProjectionMatrix(float aspect, float fov) const {
    return glm::perspective(glm::radians(fov), aspect, 0.1f, 1000.0f);
}
