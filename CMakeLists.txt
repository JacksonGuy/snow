cmake_minimum_required(VERSION 3.15)
project(Snow
    VERSION 0.1
    DESCRIPTION "A networking framework for multiplayer games"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependencies
add_subdirectory(extern/enet)

# Main project files
set(SOURCE_FILES
    src/main.cpp
    src/core/utils.cpp
    src/net/packet.cpp
    src/net/server.cpp
)
add_executable(main ${SOURCE_FILES})

# Options
target_compile_options(main PRIVATE
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Debug>:
        -O0 -g3 -pg -Wall
        -fsanitize=address -fno-omit-frame-pointer
        -fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=
    >
)
target_compile_definitions(main PRIVATE
    $<$<CONFIG:Debug>:DEBUG_BUILD>
)
target_link_options(main PRIVATE
    $<$<CONFIG:Debug>: -fsanitize=address>
)

# Link
target_link_libraries(main
    PRIVATE
        enet
)
target_include_directories(main
    PRIVATE
        src
        extern/enet/include
)
