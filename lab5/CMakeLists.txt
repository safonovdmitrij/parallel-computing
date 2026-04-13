cmake_minimum_required(VERSION 3.29)
project(lab5)

set(CMAKE_CXX_STANDARD 20)

add_executable(server server/server.cpp core/thread_pool.cpp)
add_executable(client client/client.cpp)

target_link_libraries(server ws2_32)
target_link_libraries(client ws2_32)

target_sources(server PRIVATE core/thread_pool.h)