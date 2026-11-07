# Architecture

## Components

1. SnapFrame portable core: versioned C ABI, frame ordering, configuration, bypass/fallback, lifecycle and backend callbacks. It is graphics-API neutral.
2. SnapFrame Vulkan backend: records luma/pyramid, motion estimation, fusion, consistency and reconstruction work using host-provided Vulkan resources.
3. Host adapters: bind the backend to the actual renderer/compositor. X11 and Wayland are separate integrations, not one universal presenter.
4. Snap AI policy: deterministic bounded controller consuming scalar telemetry and recommending later-frame workload settings.
5. Frame telemetry recorder: passive CPU-side recorder for host-reported presentation events. It does not hook Present, infer scanout or pace frames.

## Runtime flow

~~~text
real source frame
 -> host capture/history
 -> luma + pyramid
 -> forward/backward flow (one solve per source pair)
 -> motion fusion + consistency
 -> GPU reductions / timestamps
 -> Snap AI policy (for a subsequent safe frame)
 -> phase-specific warp / candidate reconstruction
 -> host-owned output slots
 -> host queue submission and presentation
~~~

For multiplier M (2, 3 or 4), the backend may generate up to M-1 distinct chronological phases. It must not repeat one midpoint to fake ×3/×4. Motion estimation is reused across phases for the same source pair.

## WinNative host paths

- X11: embedded VulkanRenderer calls the SnapFrame Vulkan backend from its renderer command path. The renderer owns swapchain acquisition, output targets, submission and presentation.
- Wayland: compositor adapter wraps the portable SnapFrameBackendV1 callback API and passes compositor-owned Vulkan handles. The compositor retains FIFO presentation and WSI ownership.

Both paths use the host device/queue and keep graphics synchronization outside SnapFrame and Snap AI. See [INTEGRATION_WINNATIVE.md](INTEGRATION_WINNATIVE.md).

## Snap AI signals

The WinNative Vulkan backend asynchronously reduces existing flow/consistency data into mean normalized flow magnitude, magnitude deviation (complexity proxy), consistency-weighted confidence and a confidence/consistency risk proxy. GPU stage timestamps provide workload timing. The readback is delayed through a bounded ring and consumed only when the associated query slot is complete.

Magnitude deviation is not full edge-aware vector complexity. The risk value is not calibrated artifact probability. Timestamp pressure is not hardware utilization or thermal state. Unavailable features must remain invalid, not be replaced with fabricated zeroes or perfect confidence.

## WSI and frame delivery

Generated compute completion, swapchain acquisition, successful present and physical scanout are different events. A generated frame must not be counted as displayed merely because synthesis completed.

X11 and Wayland now use bounded acquisition waits: one first generated slot may wait within a refresh-aware budget; additional generated slots use a short best-effort wait. This avoids serial long waits but does not guarantee delivery. WSI outcomes are not yet a complete Snap AI input; closed-loop delivery adaptation remains planned.

## Ownership and failure

- Host owns device, queue, command buffers, swapchain, present semaphores, output slots and presentation.
- Backend owns only private pipelines, descriptors, samplers, intermediate images and memory.
- Borrowed host handles are valid only for the documented callback/lifetime.
- No private queue submission, synchronous GPU wait, full-frame CPU readback or unbounded render-thread allocation.
- On invalid telemetry, backend failure, device loss or unsupported formats, bypass generation and retain the real frame.
- Reinitialize backend resources only at safe host device/context lifecycle points.

Policy calls are serialized per handle. The frame telemetry recorder uses a mutex and bounded storage; it is not lock-free. Snapshot sorting/allocation belongs on a diagnostics thread, never a latency-sensitive present callback.
