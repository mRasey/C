cmake_minimum_required(VERSION 3.6)
project(TestFunc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp test.c test.h)
add_executable(TestFunc ${SOURCE_FILES})