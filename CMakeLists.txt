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

include_directories(${PROJECT_SOURCE_DIR}/)

add_compile_options(-Wall -std=c++17 -lpthread -Wno-unused-parameter -O3)

link_libraries(pthread)

function (gen_static_lib lib_name)
    aux_source_directory(${PROJECT_SOURCE_DIR}/${lib_name} SRC_FILES)
    file(GLOB INCLUDES_FILES "./${lib_name}/*.hpp" "./${lib_name}/*.h")
    add_library(${lib_name} STATIC ${SRC_FILES})
    install(FILES ${INCLUDES_FILES} DESTINATION ${NETLIB_DIR}/include/${lib_name})
    install(TARGETS ${lib_name} ARCHIVE DESTINATION ${NETLIB_DIR}/lib)
endfunction()

gen_static_lib(utils)
gen_static_lib(log)
gen_static_lib(task_coroutine)
gen_static_lib(json)
gen_static_lib(context)
gen_static_lib(http)