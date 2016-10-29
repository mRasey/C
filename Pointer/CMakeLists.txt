cmake_minimum_required(VERSION 3.6)
project(Pointer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES invertsub.c)
add_executable(Pointer ${SOURCE_FILES})