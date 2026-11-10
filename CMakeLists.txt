
cmake_minimum_required(VERSION 3.16)
project(MyGame C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(raylib REQUIRED)

set(ASSET "${CMAKE_CURRENT_SOURCE_DIR}/assets/chess_sheet.png")
set(ASSET_OBJ "${CMAKE_CURRENT_BINARY_DIR}/chess_sheet.o")

add_custom_command(
    OUTPUT "${ASSET_OBJ}"
    COMMAND "${CMAKE_LINKER}" -r -b binary
            chess_sheet.png
            -o "${ASSET_OBJ}"
    DEPENDS "${ASSET}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

set_source_files_properties("${ASSET_OBJ}" PROPERTIES
    EXTERNAL_OBJECT TRUE
    GENERATED TRUE
)

add_executable(chess WIN32
    renderer.c
    engine.c
    "${ASSET_OBJ}"
)

target_link_libraries(chess PRIVATE raylib)
target_link_options(chess PRIVATE -mwindows)
