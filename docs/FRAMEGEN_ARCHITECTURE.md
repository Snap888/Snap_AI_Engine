# SnapFrame architecture

## Layers

1. **Portable core** (`framegen/src/snap_frame.cpp`): validates configuration and frame metadata, serializes frame processing, handles bypass and scene-cut history reset, dispatches a backend callback, and tracks host-visible statistics. It contains no Vulkan, DXVK, SDL, Android or Win32 headers.
2. **Backend ABI** (`framegen/include/snap_frame/snap_frame.h`): versioned C ABI. The backend owns all graphics API objects and synchronization.
3. **DXVK backend (migration source)** (`framegen/backends/dxvk/legacy`): original Skyrim SnapFrame implementation. It directly uses DXVK internals and must be refactored into the new backend vtable before it is a fully portable backend.
4. **Optional Snap AI controller** (`SnapAI::Engine`): consumes scalar telemetry and emits bounded policy recommendations. It is not part of the graphics API contract.

## Frame path

Host detects a real frame and supplies an opaque native frame token, dimensions, format, frame id and timestamp. The core validates ordering and enabled state. If enabled, it invokes `process_frame`. The backend records required GPU work using host-owned resources and synchronization, then returns stage timings and the number of generated frames accepted for presentation.

The backend must not wait for GPU completion or read back pixels in the real-frame path. Async completion and resource lifetime management remain backend responsibilities.

## Device and context lifetime

A backend instance is associated with the host graphics device/context generation. On device loss/recreation the host must shut down or reset the backend before destroying API objects, then initialize it against the new device. The portable core never retains graphics handles beyond a callback.
