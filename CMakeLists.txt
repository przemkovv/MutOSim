cmake_minimum_required(VERSION 3.9)
project(MSMutONet)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default Release)" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include(CheckIPOSupported)
check_ipo_supported()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_COLOR_MAKEFILE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(SIM_DEBUG "Enable more detailed tracing" OFF)


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(
        # using Clang
        # -g
        # -glldb
        # -pg
        -Werror
        -Weffc++
        -Weverything
        -Wpedantic
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-newline-eof
        -Wno-padded
        -Wno-exit-time-destructors
        )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # using GCC
    add_compile_options(
        # -g
        # -glldb
        # -pg
        -Wall
        -Werror
        -Weffc++
        -Wpedantic
        -Wno-padded
        -Wno-return-type
        )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # using Visual Studio C++
endif()

if(SIM_DEBUG)
    add_definitions(
        -DSIM_DEBUG=1
        )
else()
    add_definitions(
        -DSIM_DEBUG=0
        )
endif()

include(ExternalProject)

ExternalProject_Add(gsl
    GIT_REPOSITORY https://github.com/Microsoft/GSL.git
    INSTALL_DIR ${PROJECT_BINARY_DIR}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DGSL_TEST=0
    UPDATE_COMMAND ""
    )

ExternalProject_Add(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG develop
    INSTALL_DIR ${PROJECT_BINARY_DIR}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DBUILD_TESTING=0 -DJSON_MultipleHeaders=ON
    UPDATE_COMMAND ""
    )

ExternalProject_Add(type_safe
    GIT_REPOSITORY https://github.com/foonathan/type_safe
    INSTALL_DIR ${PROJECT_BINARY_DIR}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DBUILD_TESTING=0
    UPDATE_COMMAND ""
    )

ExternalProject_Add(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 4.x
    INSTALL_DIR ${PROJECT_BINARY_DIR}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DFMT_TEST=0 -DFMT_INSTALL=1
    UPDATE_COMMAND ""
    )

set(FMT_LIB fmt.a)

find_package(OpenMP)

if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost 1.66 COMPONENTS system program_options filesystem)
find_library(GMP gmp libgmp)
find_library(MPFR mpfr libmpfr)

function(append value)
    foreach(variable ${ARGN})
        set(${variable} "${${variable}} ${value}" PARENT_SCOPE)
    endforeach(variable)
endfunction()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    append("-fuse-ld=gold -Wl,--no-threads,--plugin-opt,cache-dir=${PROJECT_BINARY_DIR}/lto.cache" CMAKE_EXE_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS)
endif()

if (UNIX)
    add_custom_target(ctags ctags -R --options=${CMAKE_SOURCE_DIR}/.ctags -f ${CMAKE_SOURCE_DIR}/tags ${CMAKE_SOURCE_DIR})
endif (UNIX)

ADD_CUSTOM_TARGET(debug
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all -- -j3
    COMMENT "Switch CMAKE_BUILD_TYPE to Debug"
    )

ADD_CUSTOM_TARGET(sim_debug
    COMMAND ${CMAKE_COMMAND} -DSIM_DEBUG=1 -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all  -- -j3
    COMMENT "Switch CMAKE_BUILD_TYPE to Release"
    )

ADD_CUSTOM_TARGET(release
    COMMAND ${CMAKE_COMMAND} -DSIM_DEBUG=0 -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all -- -j3
    COMMENT "Switch CMAKE_BUILD_TYPE to Release"
    )

include_directories(SYSTEM ${PROJECT_BINARY_DIR}/include)
link_directories(${PROJECT_BINARY_DIR}/lib64 ${PROJECT_BINARY_DIR}/lib)

add_subdirectory(src)
# add_subdirectory(tests)

set_target_properties(sim PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
