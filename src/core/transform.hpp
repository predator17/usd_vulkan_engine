#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>

namespace usd_ve {

class Transform {
public:
    Transform() = default;

    void setPosition(const glm::vec3& pos);
    void setRotation(const glm::quat& rot);
    void setScale(const glm::vec3& s);
    void setScale(float s);

    glm::vec3 getPosition() const { return m_position; }
    glm::quat getRotation() const { return m_rotation; }
    glm::vec3 getScale() const { return m_scale; }

    glm::mat4 getLocalMatrix() const;
    glm::mat4 getWorldMatrix() const;

    void translate(const glm::vec3& delta);
    void rotate(const glm::quat& delta);
    void rotate(float angle, const glm::vec3& axis);
    void scale(const glm::vec3& factor);

    glm::vec3 forward() const;
    glm::vec3 up() const;
    glm::vec3 right() const;

    void setParent(Transform* parent);
    Transform* getParent() const { return m_parent; }
    const std::vector<Transform*>& getChildren() const { return m_children; }
    void addChild(Transform* child);
    void removeChild(Transform* child);

    bool isDirty() const { return m_dirty; }
    void markDirty();
    void update();

private:
    glm::vec3 m_position{0.0f};
    glm::quat m_rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 m_scale{1.0f};

    Transform* m_parent = nullptr;
    std::vector<Transform*> m_children;

    bool m_dirty = true;
    glm::mat4 m_worldMatrix{1.0f};
};

} // namespace usd_ve
