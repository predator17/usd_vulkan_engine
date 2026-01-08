# USD library

set(USD_SOURCES
    stage.cpp
    mesh.cpp
)

set(USD_HEADERS
    stage.hpp
    mesh.hpp
)

add_library(usd_ve_usd ${USD_SOURCES})
target_include_directories(usd_ve_usd PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(usd_ve_usd PUBLIC usd_ve_core)

# USD libraries (if available)
if(EXISTS "${USD_LIB_DIR}")
    target_link_libraries(usd_ve_usd PUBLIC
        ${USD_LIB_DIR}/libusd.so
        ${USD_LIB_DIR}/libusdGeom.so
        ${USD_LIB_DIR}/libusdUtils.so
        ${USD_LIB_DIR}/libarch.so
        ${USD_LIB_DIR}/libjsone.so
        ${USD_LIB_DIR}/libpxr_base.so
        pthread
    )
endif()
