cmake_minimum_required(VERSION 3.20)

project(writing_c_compiler LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(ccomp
    src/main.c
)

target_include_directories(ccomp PRIVATE include)

if (MSVC)
    target_compile_options(ccomp PRIVATE /W4 /WX)
else()
    target_compile_options(ccomp PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
