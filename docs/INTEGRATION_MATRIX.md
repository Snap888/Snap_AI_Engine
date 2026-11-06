# Integration matrix

| Host | Branch / revision | Host adapter present | CI currently validates | Runtime status |
|---|---|---|---|---|
| Skyrim_Android | `snapframe-backend-v1` | Host-specific DXVK adapter work against the custom Skyrim DXVK fork | Exact fork CMake configure/build target; full APK may depend on generated sources | Compile/device validation must be checked in the host repository |
| WinNative embedded X11 | `Snap888-patch-1`, checked at `f200bbcd` | Direct `SnapFrameVk` path in `winlator/vk/vk_renderer.c`; passive Snap AI recorder is compiled into `snap_ai` | WinNative repository owns APK flavor matrix and native tests | Code integration present; recorder not wired to successful present events; physical Turnip/Adreno validation pending; output gate is RGBA8 UNORM |
| WinNative embedded Wayland | `Snap888-patch-1`, checked at `f200bbcd` | Portable `SnapFrameBackendV1` callback adapter in `waylandcomp`; passive Snap AI recorder is compiled into `snap_ai` | WinNative repository owns APK flavor matrix and native tests | Code integration present; recorder not wired to compositor present-completion boundary; physical Turnip/Adreno validation pending |
| Winlator dev OSS | `snapframe-backend-v1` | No Winlator runtime adapter yet | Portable core tests + Android arm64 library cross-compile | Runtime/DXVK hook integration pending |
| Other DXVK ports | Host-specific | Not assumed | No host-specific CI | Requires fork/version-specific hook and ownership review |

The portable Snap AI Engine contains the policy controller and a frame-presentation telemetry recorder. A passing portable build proves only that those portable components compile and pass their unit tests. It does not prove that a host adapter reports actual display events, generated-frame delivery, GPU synchronization or physical scanout.

## Shared ownership rules

All host adapters must borrow the host's graphics device and frame resources. The host retains command-buffer submission, queue synchronization, swapchain ownership, WSI semaphores and presentation. A backend records bounded GPU work into the supplied command buffer and writes only to host-provided output slots.

The telemetry recorder is passive. Host adapters must report successful source/generated presentations with stable logical IDs and swapchain generations. It must not introduce another Present hook, queue submission, pacing loop or FG pipeline.

## Current cross-host limitations

- WinNative X11 currently requires an RGBA8 UNORM swapchain for SnapFrame output.
- WinNative Snap AI currently receives GPU timestamp timings but neutral fixed motion/confidence/artifact priors.
- WinNative has compiled the presentation recorder but has not wired it to X11/Wayland presenter events.
- Winlator has portable-library CI only; no runtime frame-generation hook is included.
- Physical Adreno/Mali validation remains host- and device-specific.
