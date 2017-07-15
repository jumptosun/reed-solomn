project(reed-solomon)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-g -O0")

include_directories(./src)
aux_source_directory(./src SRC_LIST)
add_library("vcrs" ${SRC_LIST})
