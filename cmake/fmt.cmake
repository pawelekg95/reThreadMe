include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY  https://github.com/fmtlib/fmt.git
    GIT_TAG         8.1.1
)

FetchContent_GetProperties(fmt)
if (NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
endif ()

add_library(fmt INTERFACE)

target_include_directories(fmt
        INTERFACE ${fmt_SOURCE_DIR}/include
)

target_compile_definitions(fmt
    INTERFACE FMT_HEADER_ONLY
)
