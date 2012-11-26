project(satellite)

add_executable(satellite src/main.c src/data_translation.c src/executor.c src/jnxnetwork.c src/jnxlist.c)
set(BUILD_TYPE Debug)
target_link_libraries(satellite -lmysqlclient -ldl)
add_subdirectory(src)
add_subdirectory(conf)
