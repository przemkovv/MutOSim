project(MSMutONetSim)

include_directories(${MSMutONetSim_SOURCE_DIR})

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(sim ${SRC_FILES})

add_dependencies(sim gsl)
target_link_libraries(sim PUBLIC Boost::system Boost::filesystem Boost::program_options ${CMAKE_THREAD_LIBS_INIT} type_safe nlohmann_json fmt::fmt)

add_custom_target(run COMMAND time ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sim
  COMMENT "Run with ${CMAKE_BUILD_TYPE} configuration"
  )
