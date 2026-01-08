# Core library

set(CORE_SOURCES
    assert.cpp
    bounds.cpp
    color.cpp
    logger.cpp
    timer.cpp
    transform.cpp
)

set(CORE_HEADERS
    assert.hpp
    bounds.hpp
    color.hpp
    logger.hpp
    timer.hpp
    transform.hpp
)

add_library(usd_ve_core ${CORE_SOURCES})
target_include_directories(usd_ve_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_ve_core PUBLIC spdlog::spdlog glm::glm)
