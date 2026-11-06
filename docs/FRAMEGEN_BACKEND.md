# SnapFrame backend contract

## Backend owns graphics

A backend implements `SnapFrameBackendV1` and records preprocessing, motion estimation, consistency and reconstruction passes, image barriers and temporal history. The portable core never interprets graphics handles.

The Vulkan host payload is `framegen/include/snap_frame/snap_frame_vulkan.h`. A Vulkan backend receives a `SnapFrameVulkanFrame` through `SnapFrameFrameInput.native_frame`. It borrows the compositor's physical device, device, queue, command buffer, source image/view and output slots for the callback duration.

## Vulkan ownership rules

- The host owns the `VkInstance`, `VkPhysicalDevice`, `VkDevice`, queues, command pools/buffers, swapchain and all present semaphores.
- The backend records into the supplied command buffer. It must not create a second device/queue, submit the host command buffer, wait for GPU completion, perform CPU readback, or destroy host objects.
- The backend may retain only its own pipelines, descriptors, samplers, intermediate images and memory. Release them before host device teardown.
- The host supplies output images/views and declares their initial/final layouts. The backend writes only those slots and leaves them in the declared final layout.
- Preserve the source image's declared layout and ownership.
- Version 1 forbids queue-family ownership transfers: all resources must be usable on the supplied queue family.
- Express GPU dependencies as barriers in the supplied command buffer. Never insert a private queue submission or semaphore into the host WSI chain.

The core remains Vulkan-header-free; only a Vulkan adapter includes the interop header.

## Callback rules

- `initialize`: allocate device-bound resources; fail cleanly if required capabilities are missing.
- `update_config`: apply bounded settings at a serialized host synchronization point.
- `reset_history`: clear temporal state after enable changes, scene discontinuity, resize or device recreation.
- `process_frame`: record bounded work; never block for GPU completion, perform CPU readback or allocate unbounded per-frame memory.
- `shutdown`: release backend-owned resources before device destruction.
- For multiplier `M`, emit no more than `M-1` distinct generated phases in chronological order. The core validates the count; the host presents them in that order.

## Temporal multiplier support

The portable API accepts multiplier 1 (bypass) and 2..4 (up to three generated phases). This is a scheduling/API capability, not a claim that every backend implements all multipliers. A backend must enforce its own capability limits.

## WinNative host adapters

WinNative branch `Snap888-patch-1` contains two distinct integrations:

### Embedded X11 `VulkanRenderer`

The X11 path is implemented directly in `app/src/main/cpp/winlator/vk/vk_renderer.c` and links the shared `snapframe_vk` compute backend:

- `create_snap()` binds the backend to the renderer's existing device/physical device.
- The render loop records flow processing and temporal warp phases into the existing frame command buffer.
- The renderer owns swapchain acquisition, output targets, queue submission and present synchronization.
- It supports up to three generated phases (×2/×3/×4 scheduling) and exposes Debug Flow plus real/generated frame and GPU timing diagnostics.
- Current output gate: `VK_FORMAT_R8G8B8A8_UNORM` only.

This direct X11 adapter uses the Vulkan compute backend without routing each frame through the portable `SnapFrameBackendV1` callback layer.

### Embedded Wayland compositor

The Wayland path uses the portable core and `SnapFrameBackendV1` callback in `waylandcomp/src/framegen_engine.c`. The compositor supplies the frame payload and output ring slots; its FIFO present loop owns submission and WSI synchronization.

## Snap AI in WinNative

The WinNative vendored `snap_ai.cpp` matches this repository's `src/snap_ai.cpp` at the checked revisions. The Vulkan backend collects non-blocking timestamp-query results for luma, pyramid, flow, consistency and warp stages.

Current limitation: the host supplies fixed neutral motion magnitude, motion complexity, flow confidence and artifact-risk priors. GPU-derived reductions for these signals are not yet connected. Therefore, document the present implementation as GPU-time-driven bounded Flow Scale adaptation, not full content-aware AI flow control. Policy outputs for refinement passes, reuse and artifact checks are not all connected to distinct backend shader branches yet.

## Winlator status

The `Snap888/winlator-dev-oss:snapframe-backend-v1` branch currently runs portable tests and cross-compiles the Android arm64 library. It does not yet contain a Winlator runtime/DXVK adapter. Port the adapter to the exact Winlator fork and DXVK revision; preserve that runtime's device, queue, command-buffer and WSI ownership.

## Validation status and known gaps

- Portable contract tests validate API behavior and multiplier bounds.
- WinNative host CI validates the host build/tests; inspect the current WinNative workflow run before claiming a green build.
- Physical Turnip/Adreno runtime validation remains required for WinNative X11 and Wayland separately.
- WinNative X11 currently requires RGBA8 UNORM output.
- Scene discontinuity handling in the Vulkan backend is event/frame-gap based; pixel-content scene-cut detection is not implemented.
- Snap AI currently consumes measured stage GPU times plus neutral motion/confidence/artifact priors.
- Winlator runtime adapter and device validation are pending.

Do not describe any host as hardware-validated until real-device tests confirm pacing, no black/stale frames, correct generated-frame ordering, resize handling and device-loss recovery.
