cmake_minimum_required(VERSION 3.10)
project(stcunit
    LANGUAGES C
)

set(CMAKE_C_STANDARD 99)

add_compile_options(-Wall -Wextra -pedantic -fvisibility=hidden)
add_executable(stcunit_showcase stcunit.h showcase.c)
