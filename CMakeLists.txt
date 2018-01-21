cmake_minimum_required(VERSION 3.10)
project(MSMutONet)

include(CheckIPOSupported)
check_ipo_supported()

add_compile_options(
    -std=c++17
    -g
    -glldb
    # -pg
    # -Wall
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

add_definitions(
    -DSPDLOG_FMT_EXTERNAL
    -DSIM_DEBUG=${SIM_DEBUG}
    )

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_COLOR_MAKEFILE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

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
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DBUILD_TESTING=0 -DJSON_MultipleHeaders=OFF
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
find_package(Threads REQUIRED)
find_package(Boost 1.66 COMPONENTS system program_options)
find_library(GMP gmp libgmp)
find_library(MPFR mpfr libmpfr)

function(append value)
    foreach(variable ${ARGN})
        set(${variable} "${${variable}} ${value}" PARENT_SCOPE)
    endforeach(variable)
endfunction()

append("-fuse-ld=gold -Wl,--no-threads,--plugin-opt,cache-dir=${PROJECT_BINARY_DIR}/lto.cache" CMAKE_EXE_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS)

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
