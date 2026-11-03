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

## Future adapters

DXVK, native Vulkan compositor, WinNative and other hosts implement this contract independently. A Winlator integration may use the DXVK adapter or a compositor adapter, depending on capture/present ownership. Avoid introducing a second independent present scheduler if the host already owns pacing.
