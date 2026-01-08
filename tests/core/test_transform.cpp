#include <catch2/catch.hpp>
#include <core/transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

using namespace usd_ve;

TEST_CASE("Transform: Default construction", "[core][transform]") {
    Transform transform;

    SECTION("Default position is zero") {
        REQUIRE(transform.getPosition() == glm::vec3(0.0f));
    }

    SECTION("Default rotation is identity") {
        glm::quat rotation = transform.getRotation();
        REQUIRE(rotation.w == Approx(1.0f));
        REQUIRE(rotation.x == Approx(0.0f));
        REQUIRE(rotation.y == Approx(0.0f));
        REQUIRE(rotation.z == Approx(0.0f));
    }

    SECTION("Default scale is one") {
        REQUIRE(transform.getScale() == glm::vec3(1.0f));
    }

    SECTION("Default local matrix is identity") {
        glm::mat4 matrix = transform.getLocalMatrix();
        REQUIRE(matrix[0][0] == Approx(1.0f));
        REQUIRE(matrix[1][1] == Approx(1.0f));
        REQUIRE(matrix[2][2] == Approx(1.0f));
        REQUIRE(matrix[3][3] == Approx(1.0f));
    }

    SECTION("No parent by default") {
        REQUIRE(transform.getParent() == nullptr);
    }

    SECTION("World matrix equals local matrix when no parent") {
        REQUIRE(transform.getWorldMatrix() == transform.getLocalMatrix());
    }
}

TEST_CASE("Transform: Position", "[core][transform]") {
    Transform transform;

    SECTION("Set position from vec3") {
        transform.setPosition(glm::vec3(1.0f, 2.0f, 3.0f));
        REQUIRE(transform.getPosition() == glm::vec3(1.0f, 2.0f, 3.0f));
    }

    SECTION("Set position from separate values") {
        transform.setPosition(glm::vec3(4.0f, 5.0f, 6.0f));
        REQUIRE(transform.getPosition() == glm::vec3(4.0f, 5.0f, 6.0f));
    }

    SECTION("Translate by delta") {
        transform.setPosition(glm::vec3(1.0f, 2.0f, 3.0f));
        transform.translate(glm::vec3(0.5f, 1.0f, -0.5f));
        REQUIRE(transform.getPosition() == glm::vec3(1.5f, 3.0f, 2.5f));
    }

    SECTION("Translate along axis") {
        transform.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        transform.translate(glm::vec3(2.0f, 0.0f, 0.0f));
        REQUIRE(transform.getPosition().x == Approx(2.0f));
    }
}

TEST_CASE("Transform: Rotation", "[core][transform]") {
    Transform transform;

    SECTION("Set rotation from quaternion") {
        glm::quat q = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        transform.setRotation(q);
        REQUIRE(transform.getRotation() == q);
    }

    SECTION("Rotate by angle and axis") {
        transform.rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat rotation = transform.getRotation();
        // Should be 90 degree rotation around X
        float angle = glm::angle(rotation);
        REQUIRE(angle == Approx(glm::radians(90.0f)).margin(0.01f));
    }

    SECTION("Rotate by quaternion delta") {
        glm::quat delta = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        transform.rotate(delta);
        glm::quat rotation = transform.getRotation();
        float angle = glm::angle(rotation);
        REQUIRE(angle == Approx(glm::radians(45.0f)).margin(0.01f));
    }

    SECTION("Multiple rotations accumulate") {
        transform.rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        transform.rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::quat rotation = transform.getRotation();
        float angle = glm::angle(rotation);
        REQUIRE(angle == Approx(glm::radians(180.0f)).margin(0.01f));
    }
}

TEST_CASE("Transform: Scale", "[core][transform]") {
    Transform transform;

    SECTION("Set uniform scale") {
        transform.setScale(2.0f);
        REQUIRE(transform.getScale() == glm::vec3(2.0f));
    }

    SECTION("Set scale from vec3") {
        transform.setScale(glm::vec3(1.0f, 2.0f, 3.0f));
        REQUIRE(transform.getScale() == glm::vec3(1.0f, 2.0f, 3.0f));
    }

    SECTION("Scale by vec3 factor") {
        transform.setScale(2.0f);
        transform.scale(glm::vec3(1.5f));
        REQUIRE(transform.getScale() == glm::vec3(3.0f));
    }
}

TEST_CASE("Transform: Local matrix computation", "[core][transform]") {
    Transform transform;

    SECTION("Translation matrix") {
        transform.setPosition(glm::vec3(1.0f, 2.0f, 3.0f));
        glm::mat4 matrix = transform.getLocalMatrix();
        glm::vec3 translation = glm::vec3(matrix[3]);
        REQUIRE(translation.x == Approx(1.0f));
        REQUIRE(translation.y == Approx(2.0f));
        REQUIRE(translation.z == Approx(3.0f));
    }

    SECTION("Rotation matrix") {
        transform.rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 matrix = transform.getLocalMatrix();
        // After 90 degree Y rotation, X axis becomes -Z
        REQUIRE(matrix[0][2] < -0.9f);
    }

    SECTION("Scale matrix") {
        transform.setScale(glm::vec3(2.0f, 3.0f, 4.0f));
        glm::mat4 matrix = transform.getLocalMatrix();
        REQUIRE(matrix[0][0] == Approx(2.0f));
        REQUIRE(matrix[1][1] == Approx(3.0f));
        REQUIRE(matrix[2][2] == Approx(4.0f));
    }

    SECTION("Combined TRS matrix") {
        transform.setPosition(glm::vec3(5.0f, 0.0f, 0.0f));
        transform.setScale(2.0f);
        glm::mat4 matrix = transform.getLocalMatrix();
        glm::vec3 translation = glm::vec3(matrix[3]);
        REQUIRE(translation.x == Approx(5.0f));
        REQUIRE(matrix[0][0] == Approx(2.0f));  // Scale is applied
    }
}

TEST_CASE("Transform: Direction vectors", "[core][transform]") {
    Transform transform;

    SECTION("Default forward is -Z") {
        glm::vec3 forward = transform.forward();
        REQUIRE(forward.x == Approx(0.0f));
        REQUIRE(forward.y == Approx(0.0f));
        REQUIRE(forward.z == Approx(-1.0f));
    }

    SECTION("Default up is +Y") {
        glm::vec3 up = transform.up();
        REQUIRE(up.x == Approx(0.0f));
        REQUIRE(up.y == Approx(1.0f));
        REQUIRE(up.z == Approx(0.0f));
    }

    SECTION("Default right is +X") {
        glm::vec3 right = transform.right();
        REQUIRE(right.x == Approx(1.0f));
        REQUIRE(right.y == Approx(0.0f));
        REQUIRE(right.z == Approx(0.0f));
    }

    SECTION("Forward changes with rotation") {
        transform.rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 forward = transform.forward();
        // After 90° Y rotation, forward (-Z) becomes -X in right-handed coordinate system
        REQUIRE(forward.x < -0.9f);
        REQUIRE(std::abs(forward.z) < 0.1f);
    }

    SECTION("Up changes with rotation") {
        transform.rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 up = transform.up();
        // After 90° X rotation, up (+Y) becomes +Z
        REQUIRE(up.z > 0.9f);
    }

    SECTION("Direction vectors are orthonormal") {
        transform.rotate(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 f = transform.forward();
        glm::vec3 u = transform.up();
        glm::vec3 r = transform.right();

        // All should be unit length
        REQUIRE(glm::length(f) == Approx(1.0f).margin(0.01f));
        REQUIRE(glm::length(u) == Approx(1.0f).margin(0.01f));
        REQUIRE(glm::length(r) == Approx(1.0f).margin(0.01f));

        // Should be perpendicular
        REQUIRE(glm::dot(f, u) == Approx(0.0f).margin(0.01f));
        REQUIRE(glm::dot(f, r) == Approx(0.0f).margin(0.01f));
        REQUIRE(glm::dot(u, r) == Approx(0.0f).margin(0.01f));
    }
}

TEST_CASE("Transform: Parent-child hierarchy", "[core][transform]") {
    Transform parent;
    Transform child;

    SECTION("Set parent relationship") {
        child.setParent(&parent);
        REQUIRE(child.getParent() == &parent);
    }

    SECTION("Child world matrix includes parent transform") {
        parent.setPosition(glm::vec3(5.0f, 0.0f, 0.0f));
        child.setPosition(glm::vec3(1.0f, 0.0f, 0.0f));
        child.setParent(&parent);

        glm::mat4 childWorld = child.getWorldMatrix();
        // Child's world position should be (6, 0, 0)
        glm::vec3 worldPos = glm::vec3(childWorld[3]);
        REQUIRE(worldPos.x == Approx(6.0f));
    }

    SECTION("Remove parent") {
        child.setParent(&parent);
        child.setParent(nullptr);
        REQUIRE(child.getParent() == nullptr);
    }
}

TEST_CASE("Transform: World matrix calculation", "[core][transform]") {
    Transform transform;

    SECTION("World matrix equals local when no parent") {
        transform.setPosition(glm::vec3(1.0f, 2.0f, 3.0f));
        REQUIRE(transform.getWorldMatrix() == transform.getLocalMatrix());
    }

    SECTION("World matrix is cached") {
        transform.setPosition(glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 world1 = transform.getWorldMatrix();
        glm::mat4 world2 = transform.getWorldMatrix();
        REQUIRE(world1 == world2);
    }
}

TEST_CASE("Transform: Dirty flag", "[core][transform]") {
    Transform transform;

    SECTION("Setting position marks dirty") {
        glm::mat4 before = transform.getLocalMatrix();
        transform.setPosition(glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 after = transform.getLocalMatrix();
        REQUIRE(before != after);
    }

    SECTION("Setting rotation marks dirty") {
        glm::mat4 before = transform.getLocalMatrix();
        transform.rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 after = transform.getLocalMatrix();
        REQUIRE(before != after);
    }

    SECTION("Setting scale marks dirty") {
        glm::mat4 before = transform.getLocalMatrix();
        transform.setScale(2.0f);
        glm::mat4 after = transform.getLocalMatrix();
        REQUIRE(before != after);
    }
}

TEST_CASE("Transform: Complex transformations", "[core][transform]") {
    Transform transform;

    SECTION("Rotate then translate") {
        transform.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        transform.rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        transform.translate(glm::vec3(2.0f, 0.0f, 0.0f));  // Move in local space

        glm::mat4 matrix = transform.getLocalMatrix();
        glm::vec3 worldPos = glm::vec3(matrix[3]);
        // Translation should be (2, 0, 0)
        REQUIRE(worldPos.x == Approx(2.0f));
    }

    SECTION("Scale affects translation") {
        transform.setScale(2.0f);
        transform.setPosition(glm::vec3(1.0f, 2.0f, 3.0f));
        glm::mat4 matrix = transform.getLocalMatrix();
        glm::vec3 translation = glm::vec3(matrix[3]);
        REQUIRE(translation.x == Approx(1.0f));
        REQUIRE(translation.y == Approx(2.0f));
        REQUIRE(translation.z == Approx(3.0f));
    }
}

TEST_CASE("Transform: Children management", "[core][transform]") {
    Transform parent;
    Transform child1;
    Transform child2;

    SECTION("Add children") {
        parent.addChild(&child1);
        parent.addChild(&child2);
        REQUIRE(parent.getChildren().size() == 2);
    }

    SECTION("Remove child") {
        parent.addChild(&child1);
        parent.addChild(&child2);
        parent.removeChild(&child1);
        REQUIRE(parent.getChildren().size() == 1);
        REQUIRE(parent.getChildren()[0] == &child2);
    }

    SECTION("Child parent is set when setParent is called") {
        child1.setParent(&parent);
        REQUIRE(child1.getParent() == &parent);
        // Parent should also have child in its children list
        REQUIRE(std::find(parent.getChildren().begin(), parent.getChildren().end(), &child1) != parent.getChildren().end());
    }
}
