project(MSMutONetSim)

set(TARGET_NAME mutosim_lib)
set(TARGET_NAME_EXE mutosim)

add_library(${TARGET_NAME} STATIC)
add_executable(${TARGET_NAME_EXE})

target_include_directories(${TARGET_NAME} PRIVATE ${MSMutONetSim_SOURCE_DIR})
target_include_directories(${TARGET_NAME_EXE} PRIVATE ${MSMutONetSim_SOURCE_DIR})

add_dependencies(${TARGET_NAME} type_safe sg14_ext)
add_dependencies(${TARGET_NAME_EXE} ${TARGET_NAME})

target_link_libraries(${TARGET_NAME} PRIVATE meta range-v3)
target_link_libraries(${TARGET_NAME} PRIVATE fmt::fmt fmt::fmt-header-only)
target_link_libraries(${TARGET_NAME} PRIVATE Boost::program_options Boost::system)
target_link_libraries(${TARGET_NAME} PRIVATE stdc++fs)
target_link_libraries(${TARGET_NAME} PRIVATE nlohmann_json nlohmann_json::nlohmann_json)
target_link_libraries(${TARGET_NAME} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
# target_link_libraries(${TARGET_NAME} PRIVATE ${MPFR_LIBRARIES})
# target_link_libraries(${TARGET_NAME} PRIVATE ${GMP_LIBRARY})

target_link_libraries(${TARGET_NAME_EXE} PRIVATE nlohmann_json nlohmann_json::nlohmann_json)
target_link_libraries(${TARGET_NAME_EXE} PRIVATE ${TARGET_NAME})

set_target_properties(${TARGET_NAME_EXE} ${TARGET_NAME} PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  )

check_ipo_supported(RESULT ipo_supported OUTPUT error)
if (ipo_supported)
  message(STATUS "IPO / LTO enabled")
  set_target_properties(${TARGET_NAME_EXE} ${TARGET_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
    INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE
    )
else()
  message(STATUS "IPO / LTO not supported: <${error}>")
endif()

target_sources(${TARGET_NAME_EXE} PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/mutosim.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/mutosim.h"
  )

target_sources(${TARGET_NAME} PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/calculation.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/calculation.h"
  "${CMAKE_CURRENT_LIST_DIR}/cli_options.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/cli_options.h"
  "${CMAKE_CURRENT_LIST_DIR}/cli_options_format.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/cli_options_format.h"
  "${CMAKE_CURRENT_LIST_DIR}/config.h"
  "${CMAKE_CURRENT_LIST_DIR}/logger.h"
  "${CMAKE_CURRENT_LIST_DIR}/math_utils.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/math_utils.h"
  "${CMAKE_CURRENT_LIST_DIR}/math_utils.h"
  "${CMAKE_CURRENT_LIST_DIR}/scenario_settings.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/scenario_settings.h"
  "${CMAKE_CURRENT_LIST_DIR}/topology.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/topology.h"
  "${CMAKE_CURRENT_LIST_DIR}/topology_parser.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/topology_parser.h"
  "${CMAKE_CURRENT_LIST_DIR}/traffic_class.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/traffic_class.h"
  "${CMAKE_CURRENT_LIST_DIR}/utils.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/utils.h"

  "${CMAKE_CURRENT_LIST_DIR}/model/analytical.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/model/analytical.h"
  "${CMAKE_CURRENT_LIST_DIR}/model/common.h"
  "${CMAKE_CURRENT_LIST_DIR}/model/group.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/model/group.h"
  "${CMAKE_CURRENT_LIST_DIR}/model/erlang_formula.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/model/erlang_formula.h"
  "${CMAKE_CURRENT_LIST_DIR}/model/overflow_far.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/model/overflow_far.h"
  "${CMAKE_CURRENT_LIST_DIR}/model/stream_properties.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/model/stream_properties.h"
  "${CMAKE_CURRENT_LIST_DIR}/model/stream_properties_format.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/model/stream_properties_format.h"
  "${CMAKE_CURRENT_LIST_DIR}/model/test.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/model/test.h"
  "${CMAKE_CURRENT_LIST_DIR}/model/resource.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/model/resource.h"

  "${CMAKE_CURRENT_LIST_DIR}/scenarios/simple.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/scenarios/simple.h"
  "${CMAKE_CURRENT_LIST_DIR}/scenarios/single_overflow.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/scenarios/single_overflow.h"
  "${CMAKE_CURRENT_LIST_DIR}/scenarios/topology_based.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/scenarios/topology_based.h"

  "${CMAKE_CURRENT_LIST_DIR}/simulation/world.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/world.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/stats.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/stats_format.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/stats_format.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/stats.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/group.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/group.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/load.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/load.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/load_format.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/load_format.h"
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
  "${CMAKE_CURRENT_LIST_DIR}/simulation/source_stream/source_stream_format.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/source_stream/source_stream_format.h"

  "${CMAKE_CURRENT_LIST_DIR}/simulation/overflow_policy/factory.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/overflow_policy/factory.h"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/overflow_policy/overflow_policy.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/simulation/overflow_policy/overflow_policy.h"

  "${CMAKE_CURRENT_LIST_DIR}/types/types.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/types/types.h"
  "${CMAKE_CURRENT_LIST_DIR}/types/types_format.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/types/types_format.h"
  "${CMAKE_CURRENT_LIST_DIR}/types/common.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/types/common.h"
  "${CMAKE_CURRENT_LIST_DIR}/types/operations.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/types/operations.h"
  "${CMAKE_CURRENT_LIST_DIR}/types/hash.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/types/hash.h"
  "${CMAKE_CURRENT_LIST_DIR}/types/precision.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/types/precision.h"
  "${CMAKE_CURRENT_LIST_DIR}/types/parser.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/types/parser.h"
  )

# cotire(${TARGET_NAME})
