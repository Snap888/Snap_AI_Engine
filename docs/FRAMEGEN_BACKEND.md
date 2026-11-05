# SnapFrame backend contract

## Backend owns graphics

A backend implements `SnapFrameBackendV1` and is solely responsible for recording preprocessing, motion estimation, consistency and reconstruction passes, issuing image barriers, maintaining temporal history and releasing backend-owned resources. The portable core never interprets graphics handles.

The Vulkan host payload is defined by `framegen/include/snap_frame/snap_frame_vulkan.h`. A Vulkan backend receives a `SnapFrameVulkanFrame` through `SnapFrameFrameInput.native_frame`. It borrows the compositor's physical device, device, queue, command buffer, source image/view and output slots for the duration of the callback.

## Vulkan ownership rules

- The compositor owns the `VkInstance`, `VkPhysicalDevice`, `VkDevice`, queues, command pools/buffers, swapchain and all present semaphores.
- The backend must use the supplied device and record into the supplied command buffer. It must not create a second device/queue, submit the host command buffer, wait for GPU completion, perform CPU readback, or destroy host objects.
- The backend may create and retain only its own pipelines, descriptors, samplers, intermediate images and memory. Those resources must be destroyed before host device teardown.
- The host supplies each output image/view and declares its initial and required final layout. The backend must transition outputs to the declared final layout before returning.
- The backend must leave the source image in its declared source layout and must not change its ownership.
- Queue-family ownership transfers are forbidden in this v1 contract: source, command buffer and outputs must be usable on the supplied queue family.
- All GPU dependencies must be expressed as commands/barriers in the supplied command buffer. No private queue submission or semaphore may be inserted into the host's WSI chain.

The core remains Vulkan-header-free; only a Vulkan adapter includes the interop header.

## Callback rules

- `initialize`: allocate device-bound resources; fail cleanly if required capabilities are missing.
- `update_config`: apply runtime settings at a serialized host synchronization point.
- `reset_history`: clear temporal state after enable toggles, scene cuts, resize or device recreation.
- `process_frame`: record bounded work; do not block for GPU completion, perform CPU readback or allocate unbounded per-frame memory.
- `shutdown`: release all backend-owned resources before device destruction.
- For multiplier `M`, emit no more than `M-1` generated frames in chronological temporal-phase order. The core validates the count; the host presents them in that order.

## Temporal multiplier support

The portable API accepts multiplier 1 (bypass) and 2..4 (up to three generated phases). This is a scheduling/API capability, not a claim that every backend implements all multipliers. A backend must enforce its own limits and never fabricate missing phases.

## WinNative integration status

WinNative branch `Snap888-patch-1` has a compositor-owned Vulkan present loop and an LSFG/DIS bridge. The Vulkan interop ABI is now defined, but the WinNative SnapFrame runtime adapter and GPU kernels are **not yet implemented**. The remaining work is:

1. Vendor/build the portable core in the WinNative NDK build.
2. Implement the Vulkan adapter against the existing compositor device/queue/command-buffer lifecycle.
3. Port luma pyramid, bidirectional flow, consistency, occlusion confidence and distinct ×2/×3/×4 warp phases.
4. Connect generated output slots to the existing ring and FIFO present path without touching present semaphore ownership.
5. Wire Snap AI to measured per-stage GPU timings and bounded configuration updates.
6. Add device-capability, resize, scene-cut, device-loss and long-run tests.

The current LSFG/DIS path remains independent. Do not expose SnapFrame as a selectable engine until these runtime gates pass.
