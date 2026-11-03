# SnapFrame C API

Header: `framegen/include/snap_frame/snap_frame.h`.

## Create

Call `snap_frame_default_config`, fill `SnapFrameBackendV1` with version, struct size, user pointer and callbacks, then call `snap_frame_create`. Optional `initialize` runs before the handle is returned. Destroy with `snap_frame_destroy`; this calls backend `shutdown`.

## Frame processing

Call `snap_frame_process` once per real frame with monotonically increasing `frame_id`, timestamp, dimensions, format and opaque `native_frame` token. The token is never dereferenced by the core.

- `SNAP_FRAME_OK`: backend accepted the frame and returned output data.
- `SNAP_FRAME_BYPASSED`: engine is disabled or multiplier is 1.
- Negative status: invalid arguments, concurrent call, frame ordering error or backend failure.

A concurrent call returns BUSY instead of blocking. Hosts should call on their serialized render/present path.

## Hot settings

`snap_frame_set_enabled` and `snap_frame_set_quality` update configuration without recreating the engine. GPU-affecting changes require backend `update_config`; without it, those changes fail explicitly. Successful enable-state changes reset temporal history.

## ABI compatibility

Public structs include `struct_size` and API version fields where applicable. Version 1 requires the complete v1 struct size. Future minor API versions must introduce size-negotiated compatibility before extending structs.
