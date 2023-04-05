cmake_minimum_required(VERSION 3.10)

# project name
project(netlib)

# Usage:
# 1. Set the storage location for netlib header files and static lib
set(NETLIB_DIR /home/hjp/cpp/netlib)
# 2. Build
# mkdir build
# cd build
# cmake ..
# make
# make install

# install header files
function (install_header_file arg)
    list(LENGTH ARGV argv_len)
    set(i 0)
    while(i LESS ${argv_len})
        list(GET ARGV ${i} argv_value)
        file(GLOB INCLUDES_FILES "./${argv_value}/*.hpp" "./${argv_value}/*.h")
        install(FILES ${INCLUDES_FILES} DESTINATION ${NETLIB_DIR}/include/${argv_value})
        math(EXPR i "${i}+1")
    endwhile()
endfunction()

install_header_file(utils log task_coroutine json context net)

# source *.cpp
file(GLOB SRC_FILES 
    "${PROJECT_SOURCE_DIR}/utils/*.cpp"
    "${PROJECT_SOURCE_DIR}/log/*.cpp"
    "${PROJECT_SOURCE_DIR}/task_coroutine/*.cpp"
    "${PROJECT_SOURCE_DIR}/json/*.cpp"
    "${PROJECT_SOURCE_DIR}/context/*.cpp"
    "${PROJECT_SOURCE_DIR}/net/*.cpp")

include_directories(${PROJECT_SOURCE_DIR}/)
add_compile_options(-Wall -std=c++17 -lpthread -Wno-unused-parameter -O3)
link_libraries(pthread)
# add static library
add_library(netlib STATIC ${SRC_FILES})
# install static library
install(TARGETS netlib ARCHIVE DESTINATION ${NETLIB_DIR}/)