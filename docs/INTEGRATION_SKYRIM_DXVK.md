# Skyrim_Android / DXVK integration

## Existing source

The imported implementation came from `Snap888/Skyrim_Android` branch `frame_gen`. Original `snapframe_host.cpp` hooks into DXVK through callbacks registered by the Skyrim DXVK fork. It owns history/luma/flow/consistency/midpoint images, GPU timestamp queries and diagnostic readback. It also asks DXVK to queue an additional WSI present.

That code is preserved in `framegen/backends/dxvk/legacy`; it directly uses DXVK internal headers and links to `dxvk_d3d9`.

## Migration sequence

1. Add portable SnapFrame core to Android native build.
2. Keep DXVK-specific frame capture/present hooks in DXVK adapter.
3. Move to per-device adapter state rather than original process-global singleton.
4. Map DXVK frame callback data to `SnapFrameFrameInput`; keep all DXVK types inside adapter.
5. Implement `SnapFrameBackendV1.process_frame` using existing luma, forward flow, backward flow, consistency and midpoint warp passes.
6. Report generated count only after DXVK present hook accepts midpoint image.
7. Add device-destroy/reset hooks and release all owned objects before DXVK allocator teardown.
8. Keep original real-frame present as fallback for every error.
9. Integrate Snap AI only after scalar telemetry can be sampled without CPU readback.

## Required DXVK-side callbacks

The Skyrim fork has SnapFrame-specific copy-frame, device-destroy, log and generation/present-result hooks. These are not upstream DXVK APIs. Port those hooks deliberately into the chosen DXVK revision; do not copy the whole DXVK tree into this repository.

The optional legacy CMake target requires a parent DXVK build defining `dxvk_d3d9`. Portable core builds without DXVK.
