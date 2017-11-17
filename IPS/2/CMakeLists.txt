cmake_minimum_required(VERSION 3.7)
project(2)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -pedantic -Wextra ")
set(SOURCE_FILES test_tmal.c tmal.h tmal.c)
add_executable(2 ${SOURCE_FILES})