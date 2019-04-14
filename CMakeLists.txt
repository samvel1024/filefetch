cmake_minimum_required(VERSION 3.13)
project(netstore C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


add_library(dto dto.h err.c err.h)
add_executable(test test.c)
add_executable(server server.c)
add_executable(client client.c)
target_link_libraries(test dto)
target_link_libraries(server dto)
target_link_libraries(client dto)