cmake_minimum_required(VERSION 3.10)
project(stcunit
    LANGUAGES C
)

set(CMAKE_C_STANDARD 99)

option(WITH_COVERAGE "Enable code coverage" OFF)
option(ALWAYS_VERBOSE "stcunit output is permanently verbose" OFF)
option(WITH_FORTIFY_SOURCE "Enable _FORTIFY_SOURCE" OFF)
set(CACHE{WITH_FORTIFY_SOURCE_LEVEL}
        TYPE STRING
        HELP "Enable _FORTIFY_SOURCE level"
        CACHE
        VALUE 3
)

option(WITH_ASAN "Enable Address Sanitizer")

add_compile_options(-Wall -Wextra -pedantic -fvisibility=hidden)

if(ALWAYS_VERBOSE)
    add_compile_definitions(DEBUG=1)
endif()

if(WITH_COVERAGE)
    set(CMAKE_C_FLAGS "")
    add_executable(stcunit_showcase_coverage stcunit.h showcase.c)
    target_compile_options(stcunit_showcase_coverage PRIVATE -g -O0 --coverage -U_FORTIFY_SOURCE)
    target_link_options(stcunit_showcase_coverage PRIVATE --coverage)
endif()

add_executable(stcunit_showcase stcunit.h showcase.c)
if (WITH_FORTIFY_SOURCE)
    target_compile_options(stcunit_showcase PRIVATE -g -O3 -fstack-protector-all)
    target_compile_definitions(stcunit_showcase PRIVATE _FORTIFY_SOURCE=${WITH_FORTIFY_SOURCE_LEVEL})
endif()

if (WITH_ASAN)
    target_compile_options(stcunit_showcase PRIVATE -g -fno-omit-frame-pointer -fsanitize=address)
    target_link_options(stcunit_showcase PRIVATE -fsanitize=address)
endif()
