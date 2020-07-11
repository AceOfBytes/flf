cmake_minimum_required(VERSION 3.16)
project(mff C)

set(CMAKE_C_STANDARD 99)

add_library(mffs SHARED src/libmff.c include/libmff.h)
add_executable(mff src/main.c include/libmff.h src/libmff.c)
include_directories(include)