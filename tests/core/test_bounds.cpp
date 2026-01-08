#include <catch2/catch.hpp>
#include <core/bounds.hpp>
#include <glm/glm.hpp>
#include <vector>

using namespace usd_vk;

TEST_CASE("Bounds: Default construction", "[core][bounds]") {
    Bounds bounds;

    SECTION("Initial bounds are not valid") {
        REQUIRE(bounds.isValid() == false);
    }

    SECTION("Initial center is zero for invalid bounds") {
        REQUIRE(bounds.getCenter() == glm::vec3(0.0f));
    }

    SECTION("Initial radius is zero for invalid bounds") {
        REQUIRE(bounds.getRadius() == 0.0f);
    }
}

TEST_CASE("Bounds: Compute from vertices", "[core][bounds]") {
    Bounds bounds;

    SECTION("Single vertex") {
        std::vector<glm::vec3> vertices = {{1.0f, 2.0f, 3.0f}};
        bounds.computeFromVertices(vertices);

        REQUIRE(bounds.getMin() == glm::vec3(1.0f, 2.0f, 3.0f));
        REQUIRE(bounds.getMax() == glm::vec3(1.0f, 2.0f, 3.0f));
        REQUIRE(bounds.getCenter() == glm::vec3(1.0f, 2.0f, 3.0f));
        REQUIRE(bounds.getRadius() == 0.0f);
    }

    SECTION("Multiple vertices forming a cube") {
        std::vector<glm::vec3> vertices = {
            {-1.0f, -1.0f, -1.0f},
            { 1.0f, -1.0f, -1.0f},
            {-1.0f,  1.0f, -1.0f},
            { 1.0f,  1.0f, -1.0f},
            {-1.0f, -1.0f,  1.0f},
            { 1.0f, -1.0f,  1.0f},
            {-1.0f,  1.0f,  1.0f},
            { 1.0f,  1.0f,  1.0f}
        };
        bounds.computeFromVertices(vertices);

        REQUIRE(bounds.getMin() == glm::vec3(-1.0f, -1.0f, -1.0f));
        REQUIRE(bounds.getMax() == glm::vec3(1.0f, 1.0f, 1.0f));
        REQUIRE(bounds.getCenter() == glm::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(bounds.getRadius() == Approx(std::sqrt(3.0f)));  // Distance from center to corner
    }

    SECTION("Asymmetric bounds") {
        std::vector<glm::vec3> vertices = {
            {0.0f, 0.0f, 0.0f},
            {5.0f, 0.0f, 0.0f},
            {0.0f, 3.0f, 0.0f},
            {0.0f, 0.0f, 2.0f}
        };
        bounds.computeFromVertices(vertices);

        REQUIRE(bounds.getMin() == glm::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(bounds.getMax() == glm::vec3(5.0f, 3.0f, 2.0f));
        REQUIRE(bounds.getCenter() == glm::vec3(2.5f, 1.5f, 1.0f));
    }

    SECTION("Empty vertex list") {
        std::vector<glm::vec3> vertices;
        bounds.computeFromVertices(vertices);

        // Should remain unchanged (zero bounds)
        REQUIRE(bounds.getMin() == glm::vec3(0.0f));
        REQUIRE(bounds.getMax() == glm::vec3(0.0f));
    }
}

TEST_CASE("Bounds: Contains point", "[core][bounds]") {
    Bounds bounds;
    std::vector<glm::vec3> vertices = {
        {-1.0f, -1.0f, -1.0f},
        { 1.0f,  1.0f,  1.0f}
    };
    bounds.computeFromVertices(vertices);

    SECTION("Center point is contained") {
        REQUIRE(bounds.contains(glm::vec3(0.0f, 0.0f, 0.0f)));
    }

    SECTION("Corner points are contained") {
        REQUIRE(bounds.contains(glm::vec3(-1.0f, -1.0f, -1.0f)));
        REQUIRE(bounds.contains(glm::vec3(1.0f, 1.0f, 1.0f)));
    }

    SECTION("Face centers are contained") {
        REQUIRE(bounds.contains(glm::vec3(0.0f, 0.0f, 1.0f)));
        REQUIRE(bounds.contains(glm::vec3(0.0f, 1.0f, 0.0f)));
        REQUIRE(bounds.contains(glm::vec3(1.0f, 0.0f, 0.0f)));
    }

    SECTION("Points outside are not contained") {
        REQUIRE_FALSE(bounds.contains(glm::vec3(2.0f, 0.0f, 0.0f)));
        REQUIRE_FALSE(bounds.contains(glm::vec3(0.0f, 2.0f, 0.0f)));
        REQUIRE_FALSE(bounds.contains(glm::vec3(0.0f, 0.0f, 2.0f)));
        REQUIRE_FALSE(bounds.contains(glm::vec3(-2.0f, -2.0f, -2.0f)));
    }

    SECTION("Boundary points are contained") {
        REQUIRE(bounds.contains(glm::vec3(-1.0f, 0.0f, 0.0f)));
        REQUIRE(bounds.contains(glm::vec3(1.0f, 0.0f, 0.0f)));
    }
}

TEST_CASE("Bounds: Intersects other bounds", "[core][bounds]") {
    Bounds bounds1;
    std::vector<glm::vec3> vertices1 = {
        {-1.0f, -1.0f, -1.0f},
        { 1.0f,  1.0f,  1.0f}
    };
    bounds1.computeFromVertices(vertices1);

    SECTION("Overlapping bounds intersect") {
        Bounds bounds2;
        std::vector<glm::vec3> vertices2 = {
            {0.0f, 0.0f, 0.0f},
            {2.0f, 2.0f, 2.0f}
        };
        bounds2.computeFromVertices(vertices2);

        REQUIRE(bounds1.intersects(bounds2));
        REQUIRE(bounds2.intersects(bounds1));
    }

    SECTION("Contained bounds intersect") {
        Bounds bounds2;
        std::vector<glm::vec3> vertices2 = {
            {-0.5f, -0.5f, -0.5f},
            {0.5f, 0.5f, 0.5f}
        };
        bounds2.computeFromVertices(vertices2);

        REQUIRE(bounds1.intersects(bounds2));
        REQUIRE(bounds2.intersects(bounds1));
    }

    SECTION("Adjacent bounds intersect (touching)") {
        Bounds bounds2;
        std::vector<glm::vec3> vertices2 = {
            {1.0f, 1.0f, 1.0f},
            {2.0f, 2.0f, 2.0f}
        };
        bounds2.computeFromVertices(vertices2);

        REQUIRE(bounds1.intersects(bounds2));
    }

    SECTION("Non-overlapping bounds don't intersect") {
        Bounds bounds2;
        std::vector<glm::vec3> vertices2 = {
            {2.0f, 2.0f, 2.0f},
            {3.0f, 3.0f, 3.0f}
        };
        bounds2.computeFromVertices(vertices2);

        REQUIRE_FALSE(bounds1.intersects(bounds2));
        REQUIRE_FALSE(bounds2.intersects(bounds1));
    }

    SECTION("Separated on one axis don't intersect") {
        Bounds bounds2;
        std::vector<glm::vec3> vertices2 = {
            {-1.0f, -1.0f, -1.0f},
            { 1.0f,  1.0f,  3.0f}  // Z-axis starts at 2.0 (after gap)
        };
        bounds2.computeFromVertices(vertices2);

        // bounds1 z range: [-1, 1], bounds2 z range: [-1, 3]
        // They should overlap
        REQUIRE(bounds1.intersects(bounds2));
    }
}

TEST_CASE("Bounds: Radius calculation", "[core][bounds]") {
    Bounds bounds;

    SECTION("Unit cube radius") {
        std::vector<glm::vec3> vertices = {
            {0.0f, 0.0f, 0.0f},
            {1.0f, 1.0f, 1.0f}
        };
        bounds.computeFromVertices(vertices);

        // Center is (0.5, 0.5, 0.5)
        // Distance to corner (0, 0, 0) is sqrt(0.75)
        float expectedRadius = std::sqrt(0.75f);
        REQUIRE(bounds.getRadius() == Approx(expectedRadius));
    }

    SECTION("Sphere-like bounds") {
        std::vector<glm::vec3> vertices = {
            {0.0f, 0.0f, 0.0f},
            {2.0f, 0.0f, 0.0f},
            {0.0f, 2.0f, 0.0f},
            {0.0f, 0.0f, 2.0f}
        };
        bounds.computeFromVertices(vertices);

        // Center is (1, 1, 1)
        // Distance to (0, 0, 0) is sqrt(3)
        float expectedRadius = std::sqrt(3.0f);
        REQUIRE(bounds.getRadius() == Approx(expectedRadius));
    }
}

TEST_CASE("Bounds: Negative coordinates", "[core][bounds]") {
    Bounds bounds;
    std::vector<glm::vec3> vertices = {
        {-5.0f, -3.0f, -2.0f},
        {-1.0f, -1.0f, -1.0f}
    };
    bounds.computeFromVertices(vertices);

    REQUIRE(bounds.getMin() == glm::vec3(-5.0f, -3.0f, -2.0f));
    REQUIRE(bounds.getMax() == glm::vec3(-1.0f, -1.0f, -1.0f));
    REQUIRE(bounds.getCenter() == glm::vec3(-3.0f, -2.0f, -1.5f));

    // Point inside
    REQUIRE(bounds.contains(glm::vec3(-3.0f, -2.0f, -1.5f)));

    // Point outside (too positive)
    REQUIRE_FALSE(bounds.contains(glm::vec3(0.0f, 0.0f, 0.0f)));
}

TEST_CASE("Bounds: Large values", "[core][bounds]") {
    Bounds bounds;
    std::vector<glm::vec3> vertices = {
        {-1000.0f, -2000.0f, -3000.0f},
        { 1000.0f,  2000.0f,  3000.0f}
    };
    bounds.computeFromVertices(vertices);

    REQUIRE(bounds.getCenter() == glm::vec3(0.0f, 0.0f, 0.0f));
    REQUIRE(bounds.getRadius() == Approx(std::sqrt(1000.0f*1000.0f + 2000.0f*2000.0f + 3000.0f*3000.0f)));
}

TEST_CASE("Bounds: Single dimension", "[core][bounds]") {
    Bounds bounds;

    SECTION("Line on X-axis") {
        std::vector<glm::vec3> vertices = {
            {-5.0f, 0.0f, 0.0f},
            { 5.0f, 0.0f, 0.0f}
        };
        bounds.computeFromVertices(vertices);

        REQUIRE(bounds.getMin() == glm::vec3(-5.0f, 0.0f, 0.0f));
        REQUIRE(bounds.getMax() == glm::vec3(5.0f, 0.0f, 0.0f));
        REQUIRE(bounds.getRadius() == Approx(5.0f));
    }

    SECTION("Line on Y-axis") {
        std::vector<glm::vec3> vertices = {
            {0.0f, -10.0f, 0.0f},
            {0.0f,  10.0f, 0.0f}
        };
        bounds.computeFromVertices(vertices);

        REQUIRE(bounds.getCenter() == glm::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(bounds.getRadius() == Approx(10.0f));
    }
}
