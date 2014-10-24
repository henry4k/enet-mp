cmake_minimum_required(VERSION 2.6)

project(enet-mp)

set(ENET_DEPENDENCY "libenet >= 1.2")

option(BUILD_SHARED_LIBS "Build enet-mp as shared library." OFF)

if(UNIX)
    if(BUILD_SHARED_LIBS)
        add_definitions(-fvisibility=hidden)
    endif()
endif()

if(UNIX)
    set(PKG_DEPS "${ENET_DEPENDENCY}")
    set(PKG_LIBS "")
    set(LIB_NAME enet-mp)
    configure_file(${CMAKE_SOURCE_DIR}/enet-mp.pc.in
                   ${CMAKE_BINARY_DIR}/enet-mp.pc @ONLY)
    install(FILES ${CMAKE_BINARY_DIR}/enet-mp.pc
            DESTINATION lib${LIB_SUFFIX}/pkgconfig)
endif()

add_subdirectory(src)
add_subdirectory(example)