cmake_minimum_required(VERSION 3.16)
project(MyGame C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(raylib REQUIRED)

add_executable(game renderer.c)
target_link_libraries(game PRIVATE raylib)
