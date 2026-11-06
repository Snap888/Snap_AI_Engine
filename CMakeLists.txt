cmake_minimum_required(VERSION 3.16)
project(SnapAIEngine VERSION 0.1.0 LANGUAGES C CXX)

option(BUILD_SHARED_LIBS "Build shared library by default" ON)
option(SNAP_AI_BUILD_TESTS "Build dependency-free unit tests" ON)
option(SNAP_AI_BUILD_EXAMPLE "Build minimal host example" ON)

add_library(snap_ai_engine src/snap_ai.cpp src/frame_telemetry.cpp)
add_library(SnapAI::Engine ALIAS snap_ai_engine)
target_include_directories(snap_ai_engine PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(snap_ai_engine PUBLIC cxx_std_17)
target_compile_definitions(snap_ai_engine PRIVATE SNAP_AI_ENGINE_BUILD=1)
set_target_properties(snap_ai_engine PROPERTIES OUTPUT_NAME snap_ai_engine
  CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES POSITION_INDEPENDENT_CODE YES)

if(MSVC)
  target_compile_options(snap_ai_engine PRIVATE /W4 /permissive-)
else()
  target_compile_options(snap_ai_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(SNAP_AI_BUILD_TESTS)
  enable_testing()
  add_executable(snap_ai_tests tests/test_snap_ai.cpp)
  target_link_libraries(snap_ai_tests PRIVATE SnapAI::Engine)
  add_test(NAME snap_ai_tests COMMAND snap_ai_tests)

  add_executable(snap_ai_frame_telemetry_tests tests/test_frame_telemetry.cpp)
  target_link_libraries(snap_ai_frame_telemetry_tests PRIVATE SnapAI::Engine)
  add_test(NAME snap_ai_frame_telemetry_tests COMMAND snap_ai_frame_telemetry_tests)
endif()

if(SNAP_AI_BUILD_EXAMPLE)
  add_executable(snap_ai_example examples/minimal_host.cpp)
  target_link_libraries(snap_ai_example PRIVATE SnapAI::Engine)
endif()

include(GNUInstallDirs)
install(TARGETS snap_ai_engine EXPORT SnapAITargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Backend-neutral SnapFrame frame-generation engine.
option(SNAP_FRAME_BUILD_TESTS "Build SnapFrame portable core tests" ON)
add_subdirectory(framegen)
