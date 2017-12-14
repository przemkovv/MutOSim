project(MSMutONetSim)

include_directories(${MSMutONetSim_SOURCE_DIR})

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(sim ${SRC_FILES})

target_link_libraries(sim PUBLIC ${Boost_LIBRARIES} ${FMT} ${CMAKE_THREAD_LIBS_INIT})

add_custom_target(run COMMAND time ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sim
  COMMENT "Run with ${CMAKE_BUILD_TYPE} configuration"
  )
