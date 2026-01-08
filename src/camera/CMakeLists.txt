# Camera library

set(CAMERA_SOURCES
    camera.cpp
)

set(CAMERA_HEADERS
    camera.hpp
)

add_library(usd_vk_camera ${CAMERA_SOURCES} ${CAMERA_HEADERS})
target_include_directories(usd_vk_camera PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_vk_camera PUBLIC usd_vk_core glm::glm)
