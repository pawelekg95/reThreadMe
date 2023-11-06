include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY  https://github.com/catchorg/Catch2.git
        GIT_TAG         v3.1.0
)

FetchContent_MakeAvailable(Catch2)

target_compile_definitions(Catch2
        INTERFACE CATCH_CONFIG_MAIN)
