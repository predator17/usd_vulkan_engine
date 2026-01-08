# Camera library

add_library(usd_vk_camera INTERFACE)
target_include_directories(usd_vk_camera INTERFACE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_vk_camera INTERFACE usd_vk_core glm::glm)
