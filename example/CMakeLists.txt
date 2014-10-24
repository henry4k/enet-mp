include_directories(../src)

add_executable(server server.c shared.h)
target_link_libraries(server enet-mp)

add_executable(client client.c shared.h)
target_link_libraries(client enet-mp)