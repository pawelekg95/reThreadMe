cmake_minimum_required(VERSION 3.14)

set(EXECUTABLE_NAME test${EXE_EXTENSION})

add_executable(${EXECUTABLE_NAME}
        test.cpp
        )

target_link_libraries(${EXECUTABLE_NAME}
        PUBLIC Catch2::Catch2WithMain fmt rethreadme
        )
