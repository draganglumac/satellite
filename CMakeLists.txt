project(satellite)

add_executable(satellite src/main.c src/responder.c src/jnxsql_interface.c)
set(BUILD_TYPE Debug)
target_link_libraries(satellite -ljnxc -lmysqlclient -ldl)
add_subdirectory(src)
add_subdirectory(conf)


INSTALL(PROGRAMS build/satellite DESTINATION bin)
