cmake_minimum_required(VERSION 3.14)

find_package(Threads)

add_library(rethreadme INTERFACE)

target_include_directories(rethreadme
    INTERFACE include)

target_link_libraries(rethreadme
    INTERFACE ${CMAKE_THREAD_LIBS_INIT})
