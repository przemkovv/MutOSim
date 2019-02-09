cmake_minimum_required(VERSION 3.13)

project(MSMutONet)


#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYjkPES)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default Release)" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")


include(CheckIPOSupported)
check_ipo_supported()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_COLOR_MAKEFILE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options(
    # using Clang
    # -g
    # -glldb
    # -pg
    # -stdlib=libc++
    -march=native
    -Werror
    -Weffc++
    -Weverything
    -Wpedantic
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-newline-eof
    -Wno-padded
    -Wno-exit-time-destructors
    -fcolor-diagnostics
    -fdiagnostics-show-template-tree
    )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  add_compile_options(
    # -g
    # -ggdb
    # -pg
    -Wall
    -Wextra
    -march=native
    -Werror
    -Weffc++
    -Wpedantic
    -Wno-padded
    -Wno-return-type
    -fdiagnostics-color=always
    -fdiagnostics-show-template-tree
    )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
endif()

include(ExternalProject)

ExternalProject_Add(type_safe
    GIT_REPOSITORY https://github.com/foonathan/type_safe
    INSTALL_DIR ${PROJECT_BINARY_DIR}
    GIT_TAG 4258afeecbc74f6b8ea7df9b4cff5b867b4aab4c
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DTYPE_SAFE_BUILD_TEST_EXAMPLE=OFF
    UPDATE_COMMAND ""
)

find_package(Boost REQUIRED system program_options filesystem )
find_package(fmt CONFIG REQUIRED)
find_package(MPFR 4.0.1 MODULE REQUIRED)
find_package(range-v3 CONFIG REQUIRED)
find_library(GMP_LIBRARY NAMES gmp mpir)

find_package(OpenMP)
if (OPENMP_FOUND)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

function(append value)
  foreach(variable ${ARGN})
    set(${variable} "${${variable}} ${value}" PARENT_SCOPE)
  endforeach(variable)
endfunction()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  append("-fuse-ld=gold -Wl,--no-threads,--plugin-opt,cache-dir=${PROJECT_BINARY_DIR}/lto.cache" CMAKE_EXE_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS)
endif()

option(SIM_DEBUG "Enable more detailed tracing" OFF)
option(SINGLE_THREADED "Use only single thread" OFF)

if(SIM_DEBUG)
  add_definitions(
    -DSIM_DEBUG=1
    )
else()
  add_definitions(
    -DSIM_DEBUG=0
    )
endif()

if(SINGLE_THREADED)
  add_definitions(
    -DSINGLE_THREADED=1
    )
else()
  add_definitions(
    -DSINGLE_THREADED=0
    )
endif()


ADD_CUSTOM_TARGET(debug
  COMMAND ${CMAKE_COMMAND} -DSINGLE_THREADED=1 -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all -- -j3
  COMMENT "Switch CMAKE_BUILD_TYPE to Debug"
  )

ADD_CUSTOM_TARGET(sim_debug
  COMMAND ${CMAKE_COMMAND} -DSIM_DEBUG=1 -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all  -- -j3
  COMMENT "Switch CMAKE_BUILD_TYPE to Release"
  )

ADD_CUSTOM_TARGET(release
  COMMAND ${CMAKE_COMMAND} -DSINGLE_THREADED=0 -DSIM_DEBUG=0 -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all -- -j3
  COMMENT "Switch CMAKE_BUILD_TYPE to Release"
  )

include_directories(SYSTEM ${PROJECT_BINARY_DIR}/include)
link_directories(${PROJECT_BINARY_DIR}/lib64 ${PROJECT_BINARY_DIR}/lib)

add_subdirectory(src)

