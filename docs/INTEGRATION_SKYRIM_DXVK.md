# Skyrim_Android / DXVK integration

## Exact source baseline

The adapter targets `Snap888/Skyrim_Android`, branch `frame_gen`, source commit `c385a66daef920f83a86e7eaf16ff822e507e3c7`. Its mobile DXVK build is identified in `android/app/jni/dxvk/CMakeLists.txt` as dxvk-native `1.9.2b` with mobile GPU patches.

## DXVK hook points already present in that fork

- `android/app/jni/dxvk/src/d3d9/d3d9_swapchain.cpp`: `snapframe_dxvk_set_copy_hook`, generation hooks and present-result hook. The copy hook runs on the real-frame present path. The generation hook is mutually exclusive with LSFG in the existing path and schedules the midpoint as an additional WSI present.
- `android/app/jni/dxvk/src/dxvk/dxvk_context.h/.cpp`: `dispatchSnapFrame`, `dispatchSnapFrameFlow`, `dispatchSnapFrameConsistency`, and `dispatchSnapFrameWarp` record the GPU compute passes.
- `android/app/jni/dxvk/src/dxvk/dxvk_device.cpp`: device-destroy hook releases SnapFrame-owned resources while the allocator is still alive.
- `android/app/jni/snapframe/snapframe_host.cpp`: GPU resources, shader setup, temporal history, diagnostics and now the `SnapFrameBackendV1` bridge.

## Adapter flow

1. The host creates the `SnapFrame::Engine` target and builds the DXVK host adapter after `dxvk_d3d9` exists.
2. `snapframe_host_init` maps the legacy game settings into `SnapFrameEngineConfig`, creates the portable engine and installs the DXVK copy hook.
3. Each real-frame callback packages the DXVK context, source view, device, extent and format as an opaque per-call payload, then calls `snap_frame_process`.
4. The backend callback invokes the existing Vulkan/DXVK GPU pipeline. The portable core does not include DXVK headers or dereference DXVK objects.
5. DXVK's existing generation hook obtains the midpoint image and schedules it through the existing WSI path. The present-result callback records whether that generated frame was actually queued.
6. On disable, scene cut or configuration change, the backend requests history reset. Device destruction releases DXVK resources before allocator teardown.

## CMake integration

The downstream branch `Snap888/Skyrim_Android:snapframe-backend-v1` already wires the dependency in `android/app/jni/CMakeLists.txt`:

1. Build `SDL2`, `dxvk` and `lsfg` first so the exact `dxvk_d3d9` target exists.
2. Fetch the pinned Snap_AI_Engine revision `203409c67a5402526b25774b9a90fac093d21d2a`.
3. Set `SNAP_FRAME_DXVK_SOURCE_DIR` to the local `android/app/jni/dxvk` source tree.
4. Enable `SNAP_FRAME_BUILD_DXVK_LEGACY`; disable portable tests/examples for the app build.
5. The optional target compiles `snapframe_host.cpp` and links `dxvk_d3d9` plus `SnapFrame::Engine`.
6. The parent creates `snapframe_host` as an alias to `snap_frame_dxvk_legacy`, so the existing game target and runtime bootstrap retain their expected target/API names.
7. The original `runtime/snapframe_bootstrap.c` remains compiled once by the game runtime; the imported adapter target deliberately does not compile a second copy.
8. Shader generation writes `generated/snapframe_shaders.h`; the adapter searches the binary include path as well as the legacy source-relative path.

For CI, `SNAPFRAME_VALIDATION_ONLY=ON` skips the game target (which needs generated `gen/` sources) while still configuring and compiling the exact DXVK + SnapFrame adapter target. Full APK builds remain a separate manual workflow and require the generated game sources.

Do not compile the legacy adapter as a standalone library without the matching DXVK target. Do not copy private DXVK headers into the portable engine include tree.

## Compatibility and fallback

This adapter is specific to the Skyrim fork's private hooks. A different DXVK revision must port and review those hook points. The original LSFG priority/mutual-exclusion behavior must remain intact. If capture, shader dispatch, resource allocation or midpoint presentation fails, the real frame remains the fallback.

## Validation still required

No APK build or physical-device test is claimed yet for this branch. Build the downstream Android project, then test Adreno/Mali, rapid toggles, resize, scene cuts, device recreation, shutdown, GPU timings, midpoint readiness, WSI present results and audio stability. See [DXVK adapter status](DXVK_ADAPTER_STATUS.md).
