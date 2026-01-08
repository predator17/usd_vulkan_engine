# USD library

set(USD_SOURCES
    stage.cpp
    mesh.cpp
)

set(USD_HEADERS
    stage.hpp
    mesh.hpp
)

add_library(usd_vk_usd ${USD_SOURCES})
target_include_directories(usd_vk_usd PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_vk_usd PUBLIC usd_vk_core)

# USD libraries (if available)
if(EXISTS "${USD_LIB_DIR}")
    target_link_libraries(usd_vk_usd PUBLIC
        ${USD_LIB_DIR}/libusd.so
        ${USD_LIB_DIR}/libusdGeom.so
        ${USD_LIB_DIR}/libusd_sdf.so
        ${USD_LIB_DIR}/libusd_pcp.so
        ${USD_LIB_DIR}/libusd_tf.so
        ${USD_LIB_DIR}/libusd_gf.so
        ${USD_LIB_DIR}/libusd_vt.so
        ${USD_LIB_DIR}/libusd_arch.so
        ${USD_LIB_DIR}/libusd_js.so
        ${USD_LIB_DIR}/libusd_kind.so
        pthread
    )
endif()
