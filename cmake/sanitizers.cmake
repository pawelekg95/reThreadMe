if (USE_SAN)
    if (ASAN)
        list(APPEND SAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer -fsanitize-address-use-after-scope")
    endif()

    if (TSAN)
        list(APPEND SAN_FLAGS "-fsanitize=thread")
    endif()

    if (LSAN)
        list(APPEND SAN_FLAGS "-fsanitize=leak -fno-omit-frame-pointer")
    endif()

    if (UBSAN)
        list(APPEND SAN_FLAGS "-fsanitize=undefined -fno-sanitize-recover=all")
    endif()


    set(CMAKE_C_FLAGS           "${CMAKE_C_FLAGS} ${SAN_FLAGS}" CACHE INTERNAL "")
    set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} ${SAN_FLAGS}" CACHE INTERNAL "")
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${SAN_FLAGS}" CACHE INTERNAL "")

endif()
