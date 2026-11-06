# Integration matrix

| Host | Branch / revision | Host adapter present | CI currently validates | Runtime status |
|---|---|---|---|---|
| Skyrim_Android | `snapframe-backend-v1` | DXVK adapter work in progress against the custom Skyrim DXVK fork | Exact fork CMake configure/build target; full APK depends on generated sources | Compile/device validation must be checked in the host repository |
| WinNative embedded X11 | `Snap888-patch-1`, checked at `43fdd2f` | Direct `SnapFrameVk` path in `winlator/vk/vk_renderer.c` | WinNative repository owns APK flavor matrix and native tests | Code integration present; physical Turnip/Adreno validation pending; current X11 output gate is RGBA8 UNORM |
| WinNative embedded Wayland | `Snap888-patch-1`, checked at `43fdd2f` | Portable `SnapFrameBackendV1` callback adapter in `waylandcomp` | WinNative repository owns APK flavor matrix and native tests | Code integration present; physical Turnip/Adreno validation pending |
| Winlator dev OSS | `snapframe-backend-v1`, checked at `a8f095b` | No Winlator runtime adapter yet | Portable core tests + Android arm64 library cross-compile | Runtime/DXVK hook integration pending |
| Other DXVK ports | Host-specific | Not assumed | No host-specific CI | Requires fork/version-specific hook and ownership review |

A successful Snap_AI_Engine build proves only that the portable engine compiles. A host adapter build proves only source/API compatibility. Neither proves correct GPU synchronization, generated-frame delivery or present pacing on a physical device.

## Shared ownership rules

All host adapters must borrow the host's graphics device and frame resources. The host retains command-buffer submission, queue synchronization, swapchain ownership, WSI semaphores and presentation. A backend records bounded GPU work into the supplied command buffer and writes only to host-provided output slots.

## Current cross-host limitations

- WinNative X11 currently requires an RGBA8 UNORM swapchain for SnapFrame output.
- WinNative Snap AI currently receives GPU timestamp timings but neutral fixed motion/confidence/artifact priors.
- Winlator has portable-library CI only; no runtime frame-generation hook is included.
- Physical Adreno/Mali validation remains host- and device-specific.
