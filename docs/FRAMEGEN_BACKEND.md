# SnapFrame backend contract

## Backend owns graphics

A backend implements `SnapFrameBackendV1` and records preprocessing, motion estimation, consistency and reconstruction passes, image barriers and temporal history. The portable core never interprets graphics handles.

The Vulkan host payload is `framegen/include/snap_frame/snap_frame_vulkan.h`. A Vulkan backend receives a `SnapFrameVulkanFrame` through `SnapFrameFrameInput.native_frame`. It borrows the compositor's physical device, device, queue, command buffer, source image/view and output slots for the callback duration.

## Vulkan ownership rules

- The compositor owns the `VkInstance`, `VkPhysicalDevice`, `VkDevice`, queues, command pools/buffers, swapchain and all present semaphores.
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

## WinNative integration status

WinNative branch `Snap888-patch-1` now vendors the portable core and creates a real `SnapFrameBackendV1` instance whose callback wraps the compositor's `SnapFrameVk` Vulkan implementation. The host builds a stack-scoped `SnapFrameVulkanFrame` payload with the current command buffer, sampled source view and preallocated ring output slots. The callback validates that payload, records the GPU passes and returns the generated-frame count through the portable core.

The experimental Wayland/Turnip pipeline includes:

1. Half/quarter/eighth-resolution luma pyramid.
2. Bidirectional block-matching optical flow.
3. Flow consistency and an occlusion mask.
4. Distinct midpoint warp phases for ×2/×3/×4.
5. Existing compositor FIFO presentation and ring images.
6. Timestamp-query telemetry for luma, pyramid, flow, consistency and warp stages.
7. Snap AI policy updates that multiply the user's Flow Scale baseline by a bounded adaptive ratio based on measured GPU pressure.

The compositor creates the source image views for scene/effects outputs and supplies them to the backend. SnapFrame does not submit the queue, own WSI semaphores, wait on the GPU or present images itself. Resize-time resource replacement relies on the compositor's completed per-frame fence boundary; the backend does not call `DeviceWaitIdle`.

Game Settings and the in-game overlay expose the SnapFrame engine, multiplier and Flow Scale. The selector is capability-gated by Turnip storage/sample formats and compute timestamp-query support. LSFG and DIS remain separate engines.

### Validation status and known gaps

- WinNative CI now builds the APK matrix and runs standalone contract/WSI tests; the current run must pass before declaring build validation complete.
- Contract tests cover ×2/×3/×4 phase fractions, Flow Scale grid bounds, RGBA8/RGBA16F format cases, resize, format/HDR change, scene-cut reset, arm toggle, device-generation change and a static check that the backend does not own queue/WSI operations.
- Physical Turnip/Adreno runtime validation is still required.
- Scene discontinuity is currently signalled by explicit core events and a >500 ms frame gap; pixel-based scene-cut detection is not implemented.
- Snap AI currently consumes measured stage GPU times plus neutral motion/confidence priors. GPU-derived flow confidence and motion-complexity reductions are not yet wired.
- The native adapter uses the portable API for callback lifecycle/configuration/output-count validation; generated image recording remains in the WinNative Vulkan backend.

Keep the existing LSFG/DIS paths independent. Do not describe the branch as hardware-validated until real-device tests confirm pacing, no black/stale frames, and correct ×2/×3/×4 output ordering.
