# DXVK adapter implementation status

## Implemented on this branch

A concrete `SnapFrameBackendV1` bridge is now wired into the imported Skyrim DXVK host in `framegen/backends/dxvk/legacy/snapframe_host.cpp`:

- DXVK's real-frame copy hook enters `snapframeCopyFrame`.
- The wrapper builds a `SnapFrameFrameInput` with an opaque pointer to the DXVK frame arguments and calls `snap_frame_process`.
- The v1 backend callback invokes the original GPU capture/luma/forward+backward flow/consistency/midpoint-warp implementation.
- The backend maps hot configuration updates and history reset requests into the legacy host state.
- The existing DXVK generation and present-result hooks remain responsible for handing the midpoint image to WSI and confirming queued presentation.
- A mutex protects engine lifetime against callback/shutdown races; the portable core serializes config changes with frame processing.

The legacy source remains under `legacy/` because it uses DXVK internal C++ classes. It is not a portable source file and must be compiled against the exact host fork.

## Target compatibility

The source baseline is `Snap888/Skyrim_Android` branch `frame_gen`, commit `c385a66daef920f83a86e7eaf16ff822e507e3c7`. That fork identifies itself as dxvk-native `1.9.2b` with mobile-GPU patches. Relevant hook points are documented in [Skyrim DXVK integration](INTEGRATION_SKYRIM_DXVK.md).

This is not yet a generic adapter for every upstream DXVK or Winlator fork. The adapter currently reuses the original single-host/global GPU resource state; per-device multi-instance isolation remains a future hardening task.

## Required validation before calling it production-ready

- Build the optional target inside the exact Skyrim Android CMake graph with `dxvk_d3d9` already defined and `SnapFrame::Engine` added.
- Build Android arm64-v8a APK and confirm adapter symbols are linked into the expected native library.
- Run on Adreno and Mali where supported; verify capture, all four compute stages, midpoint output and present-result callbacks.
- Test rapid enable/disable, resize, scene cuts, device recreation, shutdown while frames are in flight and LSFG mutual exclusion.
- Confirm real-frame fallback on every failure and verify audio remains unaffected.
- Compare GPU timestamps and generated/queued-present counts against the pre-adapter legacy build.

The branch has not yet completed these downstream builds or physical-device tests. Portable CI does not validate DXVK private API compatibility.
