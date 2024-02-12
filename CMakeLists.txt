cmake_minimum_required(VERSION 3.28)
project(neptune C)

set(CMAKE_C_STANDARD 11)

add_executable(neptune main.c neptune.c
        neptune.h)
add_library(std SHARED
        neptune.c
        neptune.h
        std.c)
configure_file(test.np.txt test.np)