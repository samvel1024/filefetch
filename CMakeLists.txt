cmake_minimum_required(VERSION 3.11)
project(netstore C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_compile_options("-g")

add_library(util dto.h util.h util.c)
add_executable(test test.c)
add_executable(server server.c)
add_executable(client client.c)
target_link_libraries(test util)
target_link_libraries(server util)
target_link_libraries(client util)