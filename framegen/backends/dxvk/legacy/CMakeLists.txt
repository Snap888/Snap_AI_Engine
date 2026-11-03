# Optional build of the original Skyrim SnapFrame host.
# This source is intentionally isolated: it consumes DXVK internals and is not
# part of the portable SnapFrame::Engine target.
if(NOT DEFINED SNAP_FRAME_DXVK_SOURCE_DIR OR NOT TARGET dxvk_d3d9)
  message(FATAL_ERROR "Set SNAP_FRAME_DXVK_SOURCE_DIR and add the compatible dxvk_d3d9 target before this directory")
endif()
if(NOT EXISTS "${SNAP_FRAME_DXVK_SOURCE_DIR}/src/dxvk/dxvk_context.h")
  message(FATAL_ERROR "SNAP_FRAME_DXVK_SOURCE_DIR does not point at a DXVK source tree")
endif()

find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_program(SNAPFRAME_GLSLANG glslangValidator)
find_program(SNAPFRAME_SPIRV_VAL spirv-val)
if(NOT SNAPFRAME_GLSLANG OR NOT SNAPFRAME_SPIRV_VAL)
  message(FATAL_ERROR "DXVK SnapFrame build requires glslangValidator and spirv-val on the build host")
endif()
set(SNAPFRAME_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(SNAPFRAME_GENERATED_HEADER "${SNAPFRAME_GENERATED_DIR}/snapframe_shaders.h")
add_custom_command(
  OUTPUT "${SNAPFRAME_GENERATED_HEADER}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${SNAPFRAME_GENERATED_DIR}"
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}/../../../tools/compile_snapframe_shaders.py"
    --glslang "${SNAPFRAME_GLSLANG}" --spirv-val "${SNAPFRAME_SPIRV_VAL}"
    --output "${SNAPFRAME_GENERATED_HEADER}"
  DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/../../../tools/compile_snapframe_shaders.py"
    "${CMAKE_CURRENT_LIST_DIR}/../shaders/block_match_flow.comp"
    "${CMAKE_CURRENT_LIST_DIR}/../shaders/flow_consistency.comp"
    "${CMAKE_CURRENT_LIST_DIR}/../shaders/luma_downsample.comp"
    "${CMAKE_CURRENT_LIST_DIR}/../shaders/midpoint_warp.comp"
  VERBATIM)
add_custom_target(snapframe_dxvk_shaders DEPENDS "${SNAPFRAME_GENERATED_HEADER}")

add_library(snap_frame_dxvk_legacy STATIC snapframe_host.cpp)
add_dependencies(snap_frame_dxvk_legacy snapframe_dxvk_shaders)
target_include_directories(snap_frame_dxvk_legacy PUBLIC
  "${CMAKE_CURRENT_LIST_DIR}"
  "${CMAKE_CURRENT_LIST_DIR}/../../../include")
target_include_directories(snap_frame_dxvk_legacy PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(snap_frame_dxvk_legacy PRIVATE
  "${SNAPFRAME_GENERATED_DIR}"
  "${SNAP_FRAME_DXVK_SOURCE_DIR}"
  "${SNAP_FRAME_DXVK_SOURCE_DIR}/include"
  "${SNAP_FRAME_DXVK_SOURCE_DIR}/include/native"
  "${SNAP_FRAME_DXVK_SOURCE_DIR}/include/native/windows"
  "${SNAP_FRAME_DXVK_SOURCE_DIR}/include/native/directx"
  "${SNAP_FRAME_DXVK_SOURCE_DIR}/src"
  "${SNAP_FRAME_DXVK_SOURCE_DIR}/src/util"
  "${SNAP_FRAME_DXVK_SOURCE_DIR}/src/dxvk"
  "${SNAP_FRAME_DXVK_SOURCE_DIR}/src/dxbc"
  "${SNAP_FRAME_DXVK_SOURCE_DIR}/src/spirv"
  "${SNAP_FRAME_DXVK_SOURCE_DIR}/src/vulkan"
  "${SNAP_FRAME_DXVK_SOURCE_DIR}/generated")
target_compile_definitions(snap_frame_dxvk_legacy PRIVATE NOMINMAX DXVK_NATIVE _FILE_OFFSET_BITS=64)
target_compile_options(snap_frame_dxvk_legacy PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:SHELL:-include cstdint>" -Wno-everything)
if(NOT TARGET SnapFrame::Engine)
  message(FATAL_ERROR "Add the SnapFrame framegen target before enabling the DXVK adapter")
endif()
target_link_libraries(snap_frame_dxvk_legacy PUBLIC dxvk_d3d9 SnapFrame::Engine)
if(ANDROID)
  target_link_libraries(snap_frame_dxvk_legacy PUBLIC log vulkan)
endif()
