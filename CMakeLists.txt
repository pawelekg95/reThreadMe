cmake_minimum_required(VERSION 3.14)

include(cmake/toolchain.cmake)
include(cmake/fmt.cmake)
include(cmake/doxygen.cmake)
include(cmake/Catch2.cmake)
include(cmake/sanitizers.cmake)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if (USE_SAN)
    message("Sanitizers on")
endif()

project(rethreadme LANGUAGES C CXX)

add_compile_options(-Wall -Wextra -Wpedantic -Werror $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a>)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(lib)
add_subdirectory(${APP_NAME})

doxygen_add_target(${CMAKE_CURRENT_SOURCE_DIR}/lib README.md)
