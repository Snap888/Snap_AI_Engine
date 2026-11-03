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

The `framegen/backends/dxvk/legacy/CMakeLists.txt` target now requires both the matching `dxvk_d3d9` target and `SnapFrame::Engine`. In the Skyrim Android native build:

- Add the Snap_AI_Engine `framegen` directory after configuring the DXVK target (or arrange target creation before enabling the optional legacy target).
- Set `SNAP_FRAME_DXVK_SOURCE_DIR` to the checked-out DXVK source directory.
- Enable `SNAP_FRAME_BUILD_DXVK_LEGACY` for the framegen subproject.
- Ensure the host's `snapframe_host` references are linked to `snap_frame_dxvk_legacy` and `SnapFrame::Engine` exactly once.
- Keep the existing shader compilation step and generated header path consistent with the host's NDK build.

Do not compile the legacy adapter as a standalone library without the matching DXVK target. Do not copy private DXVK headers into the portable engine include tree.

## Compatibility and fallback

This adapter is specific to the Skyrim fork's private hooks. A different DXVK revision must port and review those hook points. The original LSFG priority/mutual-exclusion behavior must remain intact. If capture, shader dispatch, resource allocation or midpoint presentation fails, the real frame remains the fallback.

## Validation still required

No APK build or physical-device test is claimed yet for this branch. Build the downstream Android project, then test Adreno/Mali, rapid toggles, resize, scene cuts, device recreation, shutdown, GPU timings, midpoint readiness, WSI present results and audio stability. See [DXVK adapter status](DXVK_ADAPTER_STATUS.md).
