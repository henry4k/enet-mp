include(FindPkgConfig)

file(GLOB_RECURSE Sources *.h *.c)
set(PublicHeaders enet_mp.h)

add_library(enet-mp ${LibraryType} ${Sources})
set_target_properties(enet-mp PROPERTIES PUBLIC_HEADER ${PublicHeaders})
set_target_properties(enet-mp PROPERTIES MACOSX_RPATH ON)

pkg_check_modules(ENET REQUIRED "${ENET_DEPENDENCY}")
include_directories(${ENET_INCLUDE_DIRS})
link_directories(${ENET_LIBRARY_DIRS})
target_link_libraries(enet-mp ${ENET_LIBRARIES})

install(TARGETS enet-mp
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include)