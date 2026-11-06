# Winlator integration

Target CI branch: [`Snap888/winlator-dev-oss:snapframe-backend-v1`](https://github.com/Snap888/winlator-dev-oss/tree/snapframe-backend-v1).

**Reference revision checked:** `a8f095bf5ac844777db7ca7a32d87b5b9eea3aaa`.

## Current status

The Winlator development branch currently contains the SnapFrame backend CI workflow and documentation, but no runtime SnapFrame/DXVK adapter implementation. Its workflow validates the portable engine and Android arm64 library build; it does not demonstrate generated frames in a running Winlator game.

WinNative's embedded X11/Wayland adapter is not automatically portable to Winlator. Winlator uses a different runtime composition and DXVK version/fork may vary by port.

## Required runtime integration

1. Select the exact Winlator port, DXVK fork and DXVK revision used by the target runtime.
2. Identify the host-owned Vulkan device, queue, command buffers, source images, output targets and present synchronization.
3. Implement a host adapter for `SnapFrameBackendV1` or a narrowly scoped Vulkan backend wrapper matching the selected runtime's ownership model.
4. Borrow host Vulkan handles; do not create a second device/queue, submit host command buffers, wait synchronously for GPU completion, read pixels back to CPU, or take ownership of WSI semaphores.
5. Record generated phases on the same serialized frame/pacing path as real frames.
6. Add per-game settings, overlay enable/disable, multiplier, Flow Scale, Debug Flow and real/generated frame diagnostics.
7. Add explicit capability checks for the formats and compute/timestamp features actually used by the backend.
8. Validate resize, swapchain recreation, scene discontinuity, device loss, semaphore ownership and sustained pacing on physical Adreno and Mali devices independently.

## DXVK versus compositor integration

A DXVK adapter is appropriate when generation should cover D3D9/10/11 games routed through the selected DXVK runtime. A compositor-level adapter is preferable when generation must cover WineD3D or native Vulkan paths too. These are different integration targets; do not assume one hook covers all rendering APIs.

## Validation boundary

- Portable SnapFrame unit/contract tests: covered by the SnapFrame backend workflow.
- Android arm64 library cross-compile: covered by the Winlator dev OSS workflow.
- Winlator runtime adapter build: not implemented/validated.
- Generated-frame presentation in a running Winlator session: not implemented/validated.
- Physical Adreno/Mali runtime validation: pending.

A successful portable library build proves API/build compatibility only. Keep the runtime adapter marked pending until the target Winlator fork has its own integration code, CI build and device test.
