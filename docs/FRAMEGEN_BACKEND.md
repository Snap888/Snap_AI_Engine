# SnapFrame backend contract

## Backend owns graphics

A backend implements `SnapFrameBackendV1` and records preprocessing, motion estimation, consistency and reconstruction passes, image barriers and temporal history. The portable core never interprets graphics handles.

The Vulkan host payload is `framegen/include/snap_frame/snap_frame_vulkan.h`. A Vulkan backend receives a `SnapFrameVulkanFrame` through `SnapFrameFrameInput.native_frame`. It borrows the host's physical device, device, queue, command buffer, source image/view and output slots for the callback duration.

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
- For multiplier `M`, emit no more than `M-1` distinct generated phases in chronological order. The host presents them in that order.

## Temporal multiplier support

The portable API accepts multiplier 1 (bypass) and 2..4 (up to three generated phases). This is a scheduling/API capability, not a claim that every backend implements all multipliers. A backend must enforce its own capability limits.

## WinNative integration status

WinNative branch `Snap888-patch-1` contains two host integrations over its Vulkan compute backend:

- **Embedded X11:** `app/src/main/cpp/winlator/vk/vk_renderer.c` calls the WinNative `SnapFrameVk` backend directly from its serialized renderer command buffer and routes generated output through its own swapchain/present loop. The X11 path does not currently route each frame through the portable `SnapFrameBackendV1` callback layer.
- **Wayland:** `app/src/main/cpp/waylandcomp/src/framegen_engine.c` creates a portable-core engine and adapts the compositor's borrowed Vulkan handles into `SnapFrameVulkanFrame`.

Both paths use the renderer/compositor-owned Vulkan device and presentation synchronization. They are separate host adapters; neither creates a second Vulkan device or lets the backend own WSI semaphores/presentation.

The WinNative GPU pipeline includes:
1. Half/quarter/eighth-resolution luma pyramid.
2. Bidirectional block-matching optical flow.
3. Flow consistency and an occlusion mask.
4. Distinct temporal warp phases for ×2/×3/×4, within available output capacity.
5. Host-owned output images and the existing FIFO presentation path.
6. Non-blocking timestamp-query telemetry for luma, pyramid, flow, consistency and warp.
7. Snap AI policy updates using measured GPU pressure and bounded user Flow Scale.

The embedded X11 renderer exposes a capability probe, enable/disable controls, generated/presented frame counters and a **Debug Flow** view. Debug Flow visualizes the forward flow field and consistency/occlusion mask; both forward and backward flow are used by interpolation.

### Validation status and known gaps

- WinNative's four-flavor APK workflow run [42134152224](https://github.com/Snap888/WinNative/actions/runs/42134152224) passed on commit `43fdd2f6a3cb214e52bc41134d7e8b9a36f8504e`.
- This is build validation only; physical Turnip/Adreno runtime validation remains required.
- Contract tests cover phase fractions, flow-grid bounds, format cases, resize, format/HDR change, scene-cut reset, arm toggle and device-generation change. Static WSI checks do not prove runtime semaphore correctness.
- Scene discontinuity currently uses explicit events and a >500 ms frame gap; pixel-based scene-cut detection is not implemented.
- Snap AI receives measured stage GPU times but currently uses neutral motion/confidence/artifact priors. GPU reductions for actual flow confidence, motion complexity and artifact risk are not wired.
- Do not claim content-aware AI, pixel scene-cut detection or device-validated pacing until implemented and tested.

## Generic Winlator ports

The WinNative integration is not automatically present in every Winlator fork. A generic port needs a host adapter for its exact renderer/compositor and DXVK version. DXVK is appropriate for supported D3D9/10/11 paths; a compositor adapter is needed to cover WineD3D and native Vulkan. Keep all device, queue, command submission, WSI semaphore and presentation ownership in that host.
