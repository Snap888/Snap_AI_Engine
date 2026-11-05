# SnapFrame C API

Portable header: `framegen/include/snap_frame/snap_frame.h`.
Vulkan host interop: `framegen/include/snap_frame/snap_frame_vulkan.h`.

## Create

Call `snap_frame_default_config`, fill `SnapFrameBackendV1` with version, struct size, user pointer and callbacks, then call `snap_frame_create`. Optional `initialize` runs before the handle is returned. Destroy with `snap_frame_destroy`; this calls backend `shutdown`.

## Frame processing

Call `snap_frame_process` once per real frame with monotonically increasing `frame_id`, timestamp, dimensions, format and opaque `native_frame` token. The portable core never dereferences that token.

A Vulkan backend may interpret `native_frame` as a pointer to `SnapFrameVulkanFrame`, after validating the complete struct size and interop version. This payload contains borrowed compositor-owned Vulkan handles, a host-recording command buffer, source image and preallocated generated-frame slots. The backend records work only; the host remains responsible for submission and presentation.

## Vulkan ownership

The backend must not create a second device or queue, submit the host command buffer, wait for completion, perform readback, or destroy host-owned resources. It must preserve the source layout and transition each generated output to its declared final layout. Version 1 requires all resources to be accessible on the supplied queue family; ownership transfers are not part of this ABI.

## Status

- `SNAP_FRAME_OK`: backend accepted the frame and returned output data.
- `SNAP_FRAME_BYPASSED`: engine is disabled or multiplier is 1.
- Negative status: invalid arguments, concurrent call, frame ordering error or backend failure.

A concurrent call returns BUSY instead of blocking. Hosts should call on their serialized render/present path.

## Multipliers and temporal phases

API v1 accepts `multiplier=1..4`. Multiplier 1 bypasses processing. For multiplier M=2, 3 or 4, a backend may return up to M-1 generated frames between the previous and current source frame. The backend must produce distinct temporal phases in chronological order; repeating a midpoint is not valid ×3/×4 interpolation. The core rejects output counts greater than the configured phase budget.

This is a portable API contract, not a promise that every backend implements all multipliers. Each backend must enforce its own capability limits and may return fewer frames or bypass safely.

## Hot settings

`snap_frame_set_enabled` and `snap_frame_set_quality` update configuration without recreating the engine. GPU-affecting changes require backend `update_config`; without it, those changes fail explicitly. Successful enable-state changes reset temporal history.

## ABI compatibility

Public structs include `struct_size` and API version fields where applicable. The Vulkan interop payload has its own `SNAP_FRAME_VULKAN_INTEROP_VERSION`; it is independent from the portable core ABI.
