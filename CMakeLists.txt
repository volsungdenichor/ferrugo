cmake_minimum_required(VERSION 3.5)
project(ferrugo)

enable_testing()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(src)
add_subdirectory(tests)