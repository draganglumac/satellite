project(satellite)

add_executable(satellite main.c data_translation.c executor.c)
set(BUILD_TYPE Debug)
target_link_libraries(satellite -ljnxc -ldl)