cmake_minimum_required(VERSION 3.10)

project(example)

# Usage
# 1. install netlib
# 2. set netlib header file directory
include_directories(/home/hjp/cpp/netlib/include)
# 3. add static libraries
link_directories(/home/hjp/cpp/netlib)
link_libraries(netlib)
# 3. Build
# mkdir build
# cd build
# cmake ..
# make
# 4. Run
# ./main

include_directories(${PROJECT_SOURCE_DIR}/)

add_compile_options(-Wall -std=c++17 -lpthread -Wno-unused-parameter -O3)

link_libraries(pthread)

file(GLOB SRC_FILES 
    "${PROJECT_SOURCE_DIR}/*.cpp"
    "${PROJECT_SOURCE_DIR}/http/*.cpp"
    "${PROJECT_SOURCE_DIR}/handler/*.cpp")

add_executable(main ${SRC_FILES})