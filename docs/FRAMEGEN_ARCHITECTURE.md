# SnapFrame architecture

## Portable and host layers

- framegen/src/snap_frame.cpp: portable lifecycle/config/frame-order core.
- framegen/include/snap_frame/snap_frame.h: versioned C ABI and backend callback contract.
- framegen/include/snap_frame/snap_frame_vulkan.h: borrowed Vulkan host-resource payload.
- WinNative app/src/main/cpp/snapframe/src/snap_frame_vk.c: active Vulkan compute backend used by its host adapters.
- framegen/backends/dxvk/legacy: preserved DXVK-coupled migration/reference implementation; not the active WinNative Vulkan backend and not a generic DXVK adapter.

## WinNative integrations

### X11
The embedded VulkanRenderer calls the SnapFrame Vulkan backend directly from its serialized renderer command path. The renderer owns swapchain acquisition, output targets, queue submission, semaphores and presentation. The X11 integration does not route every frame through the portable callback core.

### Wayland
The compositor adapter in waylandcomp/src/framegen_engine.c wraps the portable SnapFrameBackendV1 API and passes compositor-owned Vulkan handles. The compositor owns FIFO presentation and WSI synchronization.

These paths share compute implementation concepts but are separate host integrations. Test and document them independently.

## Compute pipeline

The active backend includes luma downsample, pyramid preparation, block/local flow matching, motion fusion, forward/backward consistency, flow metrics, candidate warp/score/reconstruction and debug visualization. Flow is computed for a source pair and reused for phase-specific generation. Up to three generated phases are possible under ×2/×3/×4 configuration, subject to host capability and successful output delivery.

## Snap AI

Snap AI receives GPU timestamps and asynchronous GPU-reduced flow features: normalized motion magnitude, magnitude deviation, consistency-weighted confidence and a consistency-derived risk proxy. These are not all calibrated quality truths. Additional edge-aware complexity, explicit occlusion ratio, photometric residual, search-boundary saturation and WSI delivery feedback remain future work.

## WSI acquisition policy

Both host paths bound generated-slot acquisition waits. X11 first generated slot uses a refresh-aware timeout bounded to 8–16 ms; additional slots use 1 ms. Wayland first generated slot waits one refresh interval clamped to 4–16 ms (16 ms if unknown); additional slots use 1 ms. This is best-effort delivery policy, not a guarantee of display or scanout.

## Known constraints

- X11 SnapFrame output currently gates on RGBA8 UNORM swapchain format.
- Wayland has its own HDR/format negotiation path; do not infer X11 format limits or support from Wayland.
- Pixel-content scene-cut detection is not a complete replacement for host frame-gap/event history resets.
- Snap AI does not yet use a closed-loop WSI acquire/present success ratio.
- Physical Turnip/Adreno runtime validation remains required.
