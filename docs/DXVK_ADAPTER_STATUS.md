# DXVK adapter implementation status

## Scope

The target is a concrete implementation of `SnapFrameBackendV1` for the exact DXVK fork/revision used by a host. DXVK is one adapter, not the engine itself. The portable engine must remain independent of Vulkan and DXVK private headers.

## Current state on this branch

- `SnapFrameBackendV1` ABI and portable core exist in `framegen/include/snap_frame/snap_frame.h`.
- The original Skyrim DXVK GPU pipeline and four compute shaders are retained under `framegen/backends/dxvk/legacy`.
- The legacy host includes DXVK internals and Skyrim-fork-specific hook points.
- The legacy host is **not** a concrete implementation of the new backend vtable yet.
- No Android/Windows game runtime or physical GPU test has been performed on this branch.

## Required concrete adapter

| Callback | Required behavior |
|---|---|
| initialize | Validate Vulkan device/queue/format capabilities; allocate images, buffers, descriptors, pipelines and timestamp queries |
| update_config | Apply safe runtime changes; defer resource recreation to a serialized render boundary |
| reset_history | Invalidate previous-frame, luma and motion-vector history on enable, scene cut, resize and device recreation |
| process_frame | Record preprocess, forward/backward flow, consistency and midpoint-warp compute passes; submit without CPU wait/readback |
| shutdown | Release all adapter-owned objects before DXVK device/allocator teardown |

The host owns queue submission, fences/semaphores, WSI and present pacing. The adapter must not create a second competing present scheduler. The generated-frame count is reported only after the host confirms that the generated image was accepted for presentation.

## Resource and synchronization rules

1. Never assume `native_frame` is a Vulkan image unless the host ABI explicitly says so. Define a versioned DXVK host context with typed image/view/format/layout/queue metadata.
2. Do not retain transient DXVK image wrappers beyond their documented lifetime. Copy/resolve into adapter-owned history resources when needed.
3. Declare every image layout transition and queue-family ownership transfer.
4. Do not wait for GPU completion in the per-frame callback.
5. Avoid CPU pixel readback in normal operation. GPU timestamp queries may be collected asynchronously.
6. On any unsupported format, device loss, allocation failure or invalid history, return bypass and preserve the real-frame present.
7. Ensure resize and device teardown cannot race with process_frame or hot configuration.

## Integration strategy

The original legacy source is the behavioral reference for the motion pipeline and diagnostics. Port its algorithms and shader bindings into the per-device adapter in stages; do not simply rename the old global host. Compile against the exact DXVK fork used by Skyrim first, then validate other forks with separate CI jobs.

## Acceptance criteria

- The concrete adapter registers a `SnapFrameBackendV1` vtable.
- No unresolved Skyrim-only symbol is required by the portable library.
- DXVK build succeeds for Android arm64-v8a and Windows x86_64 on the selected fork.
- Tests cover callback lifecycle, resize, scene cut, disable/enable, device-loss bypass and concurrent teardown.
- GPU validation confirms barriers/layouts and generated-image lifetime.
- Device tests compare baseline vs enabled frame pacing, GPU time, generated-frame count, artifact rate and audio stability.
