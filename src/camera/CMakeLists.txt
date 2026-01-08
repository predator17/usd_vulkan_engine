# Camera library

add_library(usd_ve_camera INTERFACE)
target_include_directories(usd_ve_camera INTERFACE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_ve_camera INTERFACE usd_ve_core glm::glm)
