# Scene library

set(SCENE_SOURCES
    scene.cpp
)

set(SCENE_HEADERS
    scene.hpp
)

add_library(usd_ve_scene ${SCENE_SOURCES} ${SCENE_HEADERS})
target_include_directories(usd_ve_scene PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_ve_scene
    PUBLIC
    usd_ve_core
    usd_ve_usd
    glm::glm
)
