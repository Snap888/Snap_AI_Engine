# SnapFrame and Snap AI testing

## Automated portable tests

The root project builds the Snap AI policy tests and frame-telemetry tests, as well as SnapFrame core tests.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DSNAP_AI_BUILD_TESTS=ON -DSNAP_FRAME_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The frame telemetry test covers:
- source and generated stream separation;
- duplicate logical event rejection (including Present/Present1 wrapper scenario);
- per-stream and combined interval percentiles;
- stale swapchain generation rejection;
- generation reset and history clearing;
- invalid event rejection and diagnostic counters.

The recorder test validates deterministic CPU-side accounting only. It does not test an actual graphics API or prove that the host event boundary represents successful display delivery.

## Shader validation

The imported GLSL compute shaders must compile for Vulkan 1.1 and pass `spirv-val`. CI runs `framegen/tools/compile_snapframe_shaders.py` and validates standalone SnapFrame configuration/build/tests.

## CI matrix

The repository workflows cover:
- Linux native build and tests;
- Windows native build and tests;
- Android arm64 cross-compilation of the portable libraries;
- shader compile/validation and standalone SnapFrame build/tests.

A successful cross-compile is not a physical-device test. Host repositories own their APK build and presentation integration workflows.

## Host and device validation

- Android ARM64 NDK build of the backend and host APK.
- Startup and clean shutdown.
- Resolution change / swapchain recreation.
- Device loss and reinitialization.
- Static scene, slow pan, fast rotation, moving NPCs, foliage/particles/water, HUD/inventory/map and long-run stress.
- Compare baseline vs generation-off vs generation-on.
- Inspect flow confidence, boundary saturation, consistency, HUD ghosting and disocclusion artifacts.
- Validate generated frame is presented only after successful GPU completion and original frame fallback remains intact.
- Compare source/generated present telemetry against existing host counters and captured output. Do not use the recorder as a live FPS source until host event wiring is verified.
- Validate audio remains unchanged.

A portable mock test does not prove shader validity, Vulkan correctness, Android ABI compatibility, visual quality or display scanout timing. Record exact commit, APK, device, driver, settings and logs for every device claim. Original test report is retained in `docs/legacy/`.
