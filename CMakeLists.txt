cmake_minimum_required(VERSION 3.26)
project(sublisp C)

set(CMAKE_C_STANDARD 11)

add_executable(sublisp main.c sublisp.c
        sublisp.h)

add_library(std SHARED sublisp.h sublisp.c std.c)

configure_file(script/test.np.txt test.np)