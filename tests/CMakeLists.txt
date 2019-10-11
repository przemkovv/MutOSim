project(MSMutONetSim_tests)

cmake_minimum_required(VERSION 3.13)

set(TARGET_NAME mutosim_test)

add_executable(${TARGET_NAME})
target_include_directories(${TARGET_NAME} PRIVATE ${MSMutONetSim_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} Catch2::Catch2 mutosim_lib)

set_target_properties(${TARGET_NAME} PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  )

target_sources(${TARGET_NAME} PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/test.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/math_util_tests.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/overflow_far_tests.cpp"
  )


include(CTest)
include(Catch)
catch_discover_tests(${TARGET_NAME})


if (CMAKE_CONFIGURATION_TYPES)
  add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} 
    --force-new-ctest-process --output-on-failure 
    --build-config "$<CONFIGURATION>"
    DEPENDS ${TARGET_NAME})
else()
  add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} 
    --force-new-ctest-process --output-on-failure
    DEPENDS ${TARGET_NAME})
endif()
