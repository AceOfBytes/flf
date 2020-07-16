cmake_minimum_required(VERSION 3.16)
project(mff C)

set(CMAKE_C_STANDARD 99)

add_library(uflasher SHARED src/libuflash.c include/libuflash.h)
add_executable(ufl src/cli.c include/libuflash.h src/libuflash.c src/getcomp.h)
include_directories(include)