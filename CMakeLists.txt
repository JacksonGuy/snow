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
add_executable(main src/main.cpp)

# Link
target_link_libraries(main
    PRIVATE
        enet
)
target_include_directories(main
    PRIVATE
        extern/enet/include
)
