project(MSMutONetSim)

add_executable(sim "")
target_include_directories(sim PRIVATE ${MSMutONetSim_SOURCE_DIR})

add_dependencies(sim gsl range-v3)
target_link_libraries(sim PUBLIC
  Boost::filesystem
  Boost::program_options
  Boost::system
  ${CMAKE_THREAD_LIBS_INIT}
  fmt::fmt
  nlohmann_json
  type_safe
  )

set_target_properties(sim PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  INTERPROCEDURAL_OPTIMIZATION TRUE)

add_custom_target(run COMMAND time ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sim
  COMMENT "Run with ${CMAKE_BUILD_TYPE} configuration")

target_sources(sim PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/calculation.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/calculation.h"
  "${CMAKE_CURRENT_LIST_DIR}/config.h"
  "${CMAKE_CURRENT_LIST_DIR}/logger.h"
  "${CMAKE_CURRENT_LIST_DIR}/math_utils.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/math_utils.h"
  "${CMAKE_CURRENT_LIST_DIR}/math_utils.h"
  "${CMAKE_CURRENT_LIST_DIR}/mutosim.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/mutosim.h"
  "${CMAKE_CURRENT_LIST_DIR}/scenario_settings.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/scenario_settings.h"
  "${CMAKE_CURRENT_LIST_DIR}/topology.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/topology.h"
  "${CMAKE_CURRENT_LIST_DIR}/topology_parser.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/topology_parser.h"
  "${CMAKE_CURRENT_LIST_DIR}/traffic_class.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/traffic_class.h"
  "${CMAKE_CURRENT_LIST_DIR}/types.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/types.h"
  "${CMAKE_CURRENT_LIST_DIR}/types_parser.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/types_parser.h"
  "${CMAKE_CURRENT_LIST_DIR}/utils.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/utils.h"

  "${CMAKE_CURRENT_LIST_DIR}/model/analytical.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/model/analytical.h"
  "${CMAKE_CURRENT_LIST_DIR}/model/fredericks_hayward.h"
  "${CMAKE_CURRENT_LIST_DIR}/model/group.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/model/group.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/model/overflow_far.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/model/overflow_far.h"
  "${CMAKE_CURRENT_LIST_DIR}/model/stream_properties.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/model/stream_properties.h"

  "${CMAKE_CURRENT_LIST_DIR}/scenarios/simple.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/scenarios/simple.h"
  "${CMAKE_CURRENT_LIST_DIR}/scenarios/single_overflow.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/scenarios/single_overflow.h"
  "${CMAKE_CURRENT_LIST_DIR}/scenarios/topology_based.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/scenarios/topology_based.h"
  "${CMAKE_CURRENT_LIST_DIR}/scenarios/simple.cpp"

  "${CMAKE_CURRENT_LIST_DIR}/simulation/world.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/world.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/stats.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/stats_format.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/stats_format.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/stats.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/world.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/group.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/group.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/load.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/load.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/event.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/event_format.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/event_format.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/event.h"

  "${CMAKE_CURRENT_LIST_DIR}/simulation/source_stream/pascal.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/source_stream/pascal.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/source_stream/poisson.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/source_stream/poisson.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/source_stream/source_stream.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/source_stream/source_stream.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/source_stream/engset.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/source_stream/engset.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/source_stream/factory.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/source_stream/factory.h"

  "${CMAKE_CURRENT_LIST_DIR}/simulation/overflow_policy/factory.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/overflow_policy/factory.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/overflow_policy/overflow_policy.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/overflow_policy/overflow_policy.h"
  )
