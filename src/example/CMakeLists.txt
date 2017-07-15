project(demo)
cmake_minimum_required(VERSION 2.8)

include_directories("../../include")
link_directories(${CMAKE_SOURCE_DIR}/../../lib)

aux_source_directory(./ SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} dl)
target_link_libraries(${PROJECT_NAME} libvcrs.a)
