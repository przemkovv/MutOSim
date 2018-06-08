cmake_minimum_required(VERSION 3.9)

include("cmake/HunterGate.cmake")
HunterGate(
  URL "https://github.com/ruslo/hunter/archive/v0.22.0.tar.gz"
  SHA1 "f91a01c6e0eb53b1dc55a6442cd93580db91da07"
  LOCAL
  )
project(MSMutONet)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default Release)" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include(hunter_cmake_args)

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
    )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
endif()

include(ExternalProject)

ExternalProject_Add(gsl
  GIT_REPOSITORY https://github.com/Microsoft/GSL.git
  INSTALL_DIR ${PROJECT_BINARY_DIR}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DGSL_TEST=0
  UPDATE_COMMAND ""
  )

ExternalProject_Add(range-v3
  GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
  INSTALL_DIR ${PROJECT_BINARY_DIR}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>  -DRANGE_V3_NO_TESTING=1 -DRANGE_V3_NO_PERF=1
  UPDATE_COMMAND ""
  )
# Package ranges-v3 is a bit outdated in hunter repository
# hunter_add_package(range-v3)
# find_package(range-v3 CONFIG REQUIRED)

hunter_add_package(Boost COMPONENTS system program_options filesystem math)
find_package(Boost CONFIG REQUIRED system program_options filesystem)

hunter_add_package(type_safe)
find_package(type_safe CONFIG REQUIRED)

hunter_add_package(nlohmann_json)
find_package(nlohmann_json CONFIG REQUIRED)

hunter_add_package(fmt)
find_package(fmt CONFIG REQUIRED)


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

if (UNIX)
  add_custom_target(ctags ctags -R --options=${CMAKE_SOURCE_DIR}/.ctags -f ${CMAKE_SOURCE_DIR}/tags ${CMAKE_SOURCE_DIR})
endif (UNIX)

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

