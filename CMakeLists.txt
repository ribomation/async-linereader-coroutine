cmake_minimum_required(VERSION 3.20)

project(async-linereader-coroutine
        VERSION 1.0.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra -Wconversion -Wshadow -Werror -Wfatal-errors -fdiagnostics-all-candidates)

add_subdirectory(sync-thread)


