# Integration matrix

| Host | Branch / revision | Host adapter present | CI scope | Runtime status |
|---|---|---|---|---|
| Skyrim_Android | snapframe-backend-v1 | Host-specific DXVK adapter for the custom Skyrim fork | Exact fork CMake configure/build target; full APK may require generated sources | Compile/device status must be checked in the host repository |
| WinNative embedded X11 | Snap888-patch-1 | Direct SnapFrameVk path in winlator/vk/vk_renderer.c | WinNative owns APK flavor matrix and native tests | Code integration present; GPU flow reductions and timestamps are consumed; WSI delivery feedback into AI remains pending; physical Turnip/Adreno validation pending; output gate RGBA8 UNORM |
| WinNative embedded Wayland | Snap888-patch-1 | Portable SnapFrameBackendV1 callback adapter in waylandcomp | WinNative owns APK flavor matrix and native tests | Code integration present; flow metrics are exposed/logged by the adapter; WSI delivery feedback into AI remains pending; physical Turnip/Adreno validation pending |
| Winlator dev OSS | snapframe-backend-v1 | No Winlator runtime adapter yet | Portable core tests + Android arm64 library cross-compile | Runtime/DXVK hook integration pending |
| Other DXVK ports | Host-specific | Not assumed | No host-specific CI | Requires fork/version-specific hook and ownership review |

The portable Snap AI Engine contains the policy controller and passive frame-presentation telemetry recorder. The recorder is not equivalent to host presenter wiring. A passing portable build proves only that portable components compile and pass unit tests; it does not prove host display-event accounting, generated-frame delivery, GPU synchronization or physical scanout.

## Shared ownership rules

All host adapters borrow the host graphics device and frame resources. The host retains command-buffer submission, queue synchronization, swapchain ownership, WSI semaphores and presentation. A backend records bounded GPU work into the supplied command buffer and writes only to host-provided output slots.

The telemetry recorder is passive. Host adapters must report successful source/generated presentations with stable logical IDs and swapchain generations. It must not introduce another Present hook, queue submission, pacing loop or FG pipeline.

## Current cross-host limitations

- WinNative X11 currently requires an RGBA8 UNORM swapchain for SnapFrame output.
- WinNative Snap AI now consumes GPU timestamp telemetry and four asynchronous flow reductions: motion magnitude, magnitude deviation, consistency-weighted confidence and a confidence/consistency risk proxy. The latter two proxy semantics are documented; they are not calibrated ground truth.
- WSI acquire/present success ratio is not yet connected as a closed-loop Snap AI feature.
- WinNative presentation recorder is compiled but is not yet wired to actual X11/Wayland successful-present call sites.
- Winlator has portable-library CI only; no runtime frame-generation hook is included.
- Physical Adreno/Turnip validation remains host- and device-specific.
