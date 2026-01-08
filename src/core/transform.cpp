#include "transform.hpp"

namespace usd_vk {

void Transform::setPosition(const glm::vec3& pos) {
    m_position = pos;
    markDirty();
}

void Transform::setRotation(const glm::quat& rot) {
    m_rotation = rot;
    markDirty();
}

void Transform::setScale(const glm::vec3& s) {
    m_scale = s;
    markDirty();
}

void Transform::setScale(float s) {
    m_scale = glm::vec3(s);
    markDirty();
}

glm::mat4 Transform::getLocalMatrix() const {
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, m_position);
    m = m * glm::mat4_cast(m_rotation);
    m = glm::scale(m, m_scale);
    return m;
}

glm::mat4 Transform::getWorldMatrix() const {
    if (m_parent) {
        return m_parent->getWorldMatrix() * getLocalMatrix();
    }
    return getLocalMatrix();
}

void Transform::translate(const glm::vec3& delta) {
    m_position += delta;
    markDirty();
}

void Transform::rotate(const glm::quat& delta) {
    m_rotation = m_rotation * delta;
    markDirty();
}

void Transform::rotate(float angle, const glm::vec3& axis) {
    m_rotation = m_rotation * glm::angleAxis(angle, axis);
    markDirty();
}

void Transform::scale(const glm::vec3& factor) {
    m_scale *= factor;
    markDirty();
}

glm::vec3 Transform::forward() const {
    return m_rotation * glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 Transform::up() const {
    return m_rotation * glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 Transform::right() const {
    return m_rotation * glm::vec3(1.0f, 0.0f, 0.0f);
}

void Transform::setParent(Transform* parent) {
    if (m_parent) {
        m_parent->removeChild(this);
    }
    m_parent = parent;
    if (m_parent) {
        m_parent->addChild(this);
    }
    markDirty();
}

void Transform::addChild(Transform* child) {
    if (child && child != this) {
        m_children.push_back(child);
    }
}

void Transform::removeChild(Transform* child) {
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end()) {
        m_children.erase(it);
    }
}

void Transform::markDirty() {
    m_dirty = true;
    for (auto* child : m_children) {
        child->markDirty();
    }
}

void Transform::update() {
    if (m_dirty) {
        m_worldMatrix = getWorldMatrix();
        m_dirty = false;
    }
}

} // namespace usd_vk
