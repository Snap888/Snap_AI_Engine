# SnapFrame backend contract

## Backend owns graphics

A backend implements `SnapFrameBackendV1` and is solely responsible for importing/receiving the host real-frame resource; creating and retaining API-specific images/buffers/shaders; recording preprocessing, motion estimation, consistency and reconstruction passes; issuing barriers and queue synchronization; scheduling generated frames; and releasing resources before device/context destruction.

The portable core does not include Vulkan headers and never assumes `native_frame` is a Vulkan handle.

## Callback rules

- `initialize`: allocate device-bound resources; fail cleanly if required capabilities are missing.
- `update_config`: apply runtime settings. Recreate resources only at a safe host synchronization point; report failure if it cannot be applied.
- `reset_history`: clear temporal state after enable toggles, scene cuts, resize or device recreation.
- `process_frame`: record/submit bounded work; do not block for GPU completion, perform CPU readback or allocate unbounded per-frame memory.
- `shutdown`: release all backend-owned resources.
- For multiplier `M`, the backend may emit no more than `M-1` generated frames. Generated frames must be ordered by temporal phase between source frame N-1 and N. The core validates the count but the host is responsible for presenting them in that order.

## Temporal multiplier support

The portable API accepts multiplier 1 (bypass) and 2..4 (up to three generated phases). This is a scheduling/API capability, not a claim that every backend implements all multipliers. Each backend must advertise or enforce its own limits and must never fabricate missing phases. WinNative's current LSFG/DIS path is separate and is not a SnapFrame backend.

## Host adapters

DXVK, native Vulkan compositor, WinNative and other hosts implement this contract independently. A Winlator integration may use the DXVK adapter or a compositor adapter, depending on capture/present ownership. Avoid introducing a second independent present scheduler if the host already owns pacing.

## WinNative integration status

The WinNative branch `Snap888-patch-1` currently has an established compositor-owned Vulkan present loop and a frame-generation bridge for LSFG/DIS. It does **not** yet contain a SnapFrame Vulkan implementation. The next adapter must:

1. Build this portable core as a PIC static/shared dependency in the Android NDK build.
2. Implement `SnapFrameBackendV1` against the compositor's existing Vulkan device/queue and command-buffer lifecycle; do not create a second device or queue.
3. Use host-owned scene images and semaphores; never take ownership of the swapchain's present semaphores.
4. Port the SnapFrame optical-flow, consistency, occlusion and multi-phase warp passes to this adapter (the legacy DXVK host is not a drop-in WinNative backend).
5. Return generated images to the existing compositor present loop in strict temporal order, preserving its FIFO pacing and per-present semaphore ownership.
6. Keep LSFG/DIS selectable independently until the SnapFrame backend passes device-capability, resize, scene-cut, device-loss and long-run stress tests.

The portable API's ×4 support alone does not enable ×4 generation in WinNative; the Vulkan adapter and host present path must explicitly implement it.
