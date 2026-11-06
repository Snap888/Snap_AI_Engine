# SnapFrame architecture

## Layers

1. **Portable core** (`framegen/src/snap_frame.cpp`): validates configuration and frame metadata, serializes frame processing, handles bypass and scene-cut history reset, dispatches a backend callback, and tracks host-visible statistics. It contains no Vulkan, DXVK, SDL, Android or Win32 headers.
2. **Backend ABI** (`framegen/include/snap_frame/snap_frame.h`): versioned C ABI. The backend records graphics work using host-owned graphics objects and synchronization.
3. **Vulkan host implementation**: WinNative's `app/src/main/cpp/snapframe/src/snap_frame_vk.c` contains the Android/Turnip compute implementation used by its embedded X11 renderer and Wayland compositor adapters.
4. **DXVK migration source** (`framegen/backends/dxvk/legacy`): the original Skyrim SnapFrame implementation. It directly uses DXVK internals and is migration/reference code; it is not the Vulkan implementation currently used by WinNative and is not a universal Winlator backend.
5. **Optional Snap AI controller** (`SnapAI::Engine`): consumes scalar telemetry and emits bounded policy recommendations. It is not part of the graphics API contract.

## WinNative host adapters

The current WinNative branch `Snap888-patch-1` has two distinct host integrations:

- **Embedded X11:** `app/src/main/cpp/winlator/vk/vk_renderer.c` calls the WinNative Vulkan compute backend directly from its serialized renderer command buffer and uses renderer-owned swapchain images/presentation.
- **Wayland:** `app/src/main/cpp/waylandcomp/src/framegen_engine.c` wraps the portable `SnapFrameBackendV1` core and passes compositor-owned Vulkan handles through `SnapFrameVulkanFrame`.

Both adapters use the host's existing Vulkan device and presentation synchronization. They are not the same presenter, and the X11 path does not currently route every frame through the portable callback core.

## Frame path

The host detects a real frame and supplies dimensions, format, frame id/timestamp and borrowed native resources. The Wayland adapter additionally passes an opaque `SnapFrameVulkanFrame` token through the portable core. The Vulkan backend records preprocessing, motion estimation, consistency and reconstruction work into the host's command buffer. The host remains responsible for submission, generated-image scheduling and presentation.

The backend must not wait for GPU completion or read back pixels in the real-frame path. GPU timestamp results are collected non-blockingly after completion. Async completion and resource lifetime management remain host/backend responsibilities according to the adapter contract.

## Snap AI status

WinNative instantiates Snap AI in its Vulkan backend and applies bounded Flow Scale adjustments using measured GPU timestamp telemetry. Current motion magnitude, motion complexity, flow confidence and artifact-risk inputs are neutral fixed priors; they are not yet GPU reductions from the actual flow field. The current policy is therefore primarily GPU-pressure adaptive, not fully content-aware.

## Device and context lifetime

A backend instance is associated with the host graphics device/context generation. On device loss/recreation the host must shut down or abandon backend resources before destroying API objects, then initialize against the new device. The portable core never retains graphics handles beyond a callback.

## Generic Winlator ports

The WinNative adapters do not automatically apply to every Winlator fork. A generic port requires a host adapter for its exact renderer/compositor and DXVK fork/version. A DXVK adapter is appropriate for supported D3D9/10/11 paths; a compositor/presenter adapter is needed to cover WineD3D and native Vulkan paths. Keep device, queue, command submission, WSI semaphores and presentation owned by the selected host.
