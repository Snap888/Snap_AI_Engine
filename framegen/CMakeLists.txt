cmake_minimum_required(VERSION 3.16)
project(SnapFrameEngine VERSION 0.1.0 LANGUAGES C CXX)

option(SNAP_FRAME_BUILD_TESTS "Build SnapFrame portable core tests" ON)
option(SNAP_FRAME_BUILD_EXAMPLE "Build SnapFrame mock backend example" ON)
option(SNAP_FRAME_BUILD_DXVK_LEGACY "Build the imported DXVK-coupled host (requires DXVK source tree)" OFF)

add_library(snap_frame_engine src/snap_frame.cpp)
add_library(SnapFrame::Engine ALIAS snap_frame_engine)
target_include_directories(snap_frame_engine PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(snap_frame_engine PUBLIC cxx_std_17)
target_compile_definitions(snap_frame_engine PRIVATE SNAP_FRAME_ENGINE_BUILD=1)
set_target_properties(snap_frame_engine PROPERTIES OUTPUT_NAME snap_frame_engine
  CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES POSITION_INDEPENDENT_CODE YES)
if(MSVC)
  target_compile_options(snap_frame_engine PRIVATE /W4 /permissive-)
else()
  target_compile_options(snap_frame_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Backend-neutral SnapMotionV2 metadata/configuration core. It deliberately
# has no Vulkan/DXVK dependency; GPU algorithms live behind a host adapter.
add_library(snap_motion_v2 src/motion/v2/motion_v2.cpp)
add_library(SnapFrame::MotionV2 ALIAS snap_motion_v2)
target_include_directories(snap_motion_v2 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(snap_motion_v2 PUBLIC cxx_std_17)
target_compile_definitions(snap_motion_v2 PRIVATE SNAP_MOTION_V2_BUILD=1)
set_target_properties(snap_motion_v2 PROPERTIES OUTPUT_NAME snap_motion_v2
  CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES POSITION_INDEPENDENT_CODE YES)
if(MSVC)
  target_compile_options(snap_motion_v2 PRIVATE /W4 /permissive-)
else()
  target_compile_options(snap_motion_v2 PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(SNAP_FRAME_BUILD_DXVK_LEGACY)
  if(NOT DEFINED SNAP_FRAME_DXVK_SOURCE_DIR OR NOT TARGET dxvk_d3d9)
    message(FATAL_ERROR "SNAP_FRAME_BUILD_DXVK_LEGACY requires SNAP_FRAME_DXVK_SOURCE_DIR and existing target dxvk_d3d9. Add this project from a DXVK build.")
  endif()
  add_subdirectory(backends/dxvk/legacy)
endif()

if(SNAP_FRAME_BUILD_EXAMPLE)
  add_executable(snap_frame_mock_example ../examples/framegen_mock_backend.cpp)
  target_link_libraries(snap_frame_mock_example PRIVATE SnapFrame::Engine)
endif()

if(SNAP_FRAME_BUILD_TESTS)
  enable_testing()
  add_executable(snap_frame_tests tests/test_snap_frame.cpp)
  target_link_libraries(snap_frame_tests PRIVATE SnapFrame::Engine)
  add_test(NAME snap_frame_tests COMMAND snap_frame_tests)

  add_executable(snap_motion_v2_tests tests/test_motion_v2.cpp)
  target_link_libraries(snap_motion_v2_tests PRIVATE SnapFrame::MotionV2)
  add_test(NAME snap_motion_v2_tests COMMAND snap_motion_v2_tests)
endif()

include(GNUInstallDirs)
install(TARGETS snap_frame_engine snap_motion_v2 EXPORT SnapFrameTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
