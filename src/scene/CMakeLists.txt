# Scene library

set(SCENE_SOURCES
    scene.cpp
)

set(SCENE_HEADERS
    scene.hpp
)

add_library(usd_vk_scene ${SCENE_SOURCES} ${SCENE_HEADERS})
target_include_directories(usd_vk_scene PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_vk_scene
    PUBLIC
    usd_vk_core
    usd_vk_usd
    glm::glm
)
