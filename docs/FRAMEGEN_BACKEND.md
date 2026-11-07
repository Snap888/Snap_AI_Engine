# SnapFrame backend and host ownership contract

## Ownership

The host owns Vulkan instance/device/physical device, queues, command buffers, swapchain, output slots, WSI semaphores, submission and presentation. A Vulkan backend borrows host resources for the documented callback lifetime and owns only its private pipelines, descriptors, samplers, intermediate images and memory.

The backend must not create a second device/queue, submit the host command buffer, wait synchronously for GPU completion, perform full-frame CPU readback, destroy host objects, take over frame pacing or retain borrowed handles beyond their valid lifetime.

## Callback contract

- initialize: allocate private resources and fail cleanly if capabilities are absent.
- update_config: apply changes at a serialized safe point.
- reset_history: invalidate history after scene discontinuity, frame gap, resize, format/device/swapchain recreation or enable-state change.
- process_frame: record bounded GPU work into the supplied command buffer.
- shutdown: release backend-owned resources before host device teardown.

## Temporal phases and flow reuse

For multiplier M, emit no more than M-1 distinct generated phases in chronological order. Multiplier 1 is bypass. ×2/×3/×4 correspond to up to one/two/three generated phases. The backend should solve forward/backward flow once for a consecutive source pair and reuse it for phase-specific warp/reconstruction. Repeating one midpoint for multiple slots is invalid.

The host presents only outputs whose GPU work completed successfully. Generation count, swapchain acquire success, successful present and physical scanout are separate quantities.

## WinNative X11

The embedded VulkanRenderer calls the Vulkan backend directly. The renderer owns swapchain image acquisition, generated output targets, command submission and present. The active X11 output-format gate is RGBA8 UNORM; do not claim general HDR or RGBA16F swapchain output for this path.

Generated-slot acquire policy: first slot uses a refresh-aware timeout bounded to 8–16 ms; subsequent generated slots use a 1 ms best-effort timeout. VK_TIMEOUT is a missed opportunity, not device loss by itself.

## WinNative Wayland

The compositor adapter wraps the portable SnapFrameBackendV1 API. The compositor supplies borrowed Vulkan resources and retains FIFO presentation, queue submission and WSI semaphores. The first generated slot waits one refresh interval clamped to 4–16 ms (unknown refresh defaults to 16 ms); subsequent slots use 1 ms.

Wayland format/HDR behavior is separate from X11 and must be validated against negotiated surface formats.

## Snap AI boundary

Snap AI consumes scalar telemetry and returns bounded policy suggestions. WinNative has asynchronous reductions for motion magnitude, magnitude deviation, consistency-weighted confidence and a confidence/consistency risk proxy, plus GPU timestamps. The proxy is not calibrated artifact probability; complexity is not yet a complete edge-aware vector measure.

The AI does not own Vulkan synchronization or decide acquire/present operations. Feeding WSI delivery ratios into policy is planned, not a current closed loop. Not every decision field is wired to a separate shader control.

## Validation

Portable contract tests validate API rules and output bounds. Host CI validates only jobs actually run and pass. Physical X11 and Wayland device validation must separately confirm image quality, output ordering, acquire/present behavior, resize, format/HDR, device loss, thermal stability and fallback.
