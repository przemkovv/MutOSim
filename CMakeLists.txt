cmake_minimum_required(VERSION 3.10)
project(MSMutONet)

add_compile_options(
    -std=c++17
    -g
    -Wall
    -Werror
    -Weffc++
)

add_definitions(
    -DSPDLOG_FMT_EXTERNAL
    )

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_COLOR_MAKEFILE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(ExternalProject)

ExternalProject_Add(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    INSTALL_DIR ${PROJECT_BINARY_DIR}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DBuildTests=OFF
    UPDATE_COMMAND ""
    )

find_library(FMT fmt libfmt)
find_package(Threads REQUIRED)
find_package(Boost 1.65 COMPONENTS system program_options)

add_subdirectory(src)
# add_subdirectory(tests)
