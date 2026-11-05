# DXVK backend

This directory holds the first host-specific SnapFrame integration.

- `shaders/`: SnapFrame compute kernels.
- `legacy/`: the original DXVK-bound implementation, including the capture/present hook host and bootstrap.

## Current status

The legacy implementation is preserved and build-isolated. It still calls Skyrim-fork-specific DXVK hooks and uses DXVK internal C++ classes. It does **not** yet implement `SnapFrameBackendV1` as a concrete backend object. The portable `SnapFrame::Engine` can be built without this directory.

The shared Vulkan host payload is defined in `framegen/include/snap_frame/snap_frame_vulkan.h`. It is intentionally independent of DXVK: each host supplies its own device, command buffer, source image and output slots while retaining queue submission and WSI ownership.

Do not enable `SNAP_FRAME_BUILD_DXVK_LEGACY` in a standalone build. A parent project must first build the matching DXVK fork and expose the `dxvk_d3d9` target, then set `SNAP_FRAME_DXVK_SOURCE_DIR`.

## Required next refactor

1. Replace process-global DXVK state with an adapter instance per device/swapchain.
2. Move all Vulkan/DXVK resources behind the adapter object.
3. Implement backend callbacks: initialize, update_config, reset_history, process_frame and shutdown.
4. Map the original copy-frame hook into a stable per-frame callback.
5. Map generated midpoint image acceptance to output.generated_frames.
6. Keep all queue ownership, barriers, fences/semaphores and WSI scheduling in DXVK.
7. Add compile-time integration tests against the exact DXVK revision used by each host.
