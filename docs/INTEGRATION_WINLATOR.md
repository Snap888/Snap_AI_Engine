# Winlator / WinNative integration status

## Confirmed WinNative integration

Target application repository: [Snap888/WinNative](https://github.com/Snap888/WinNative), branch [Snap888-patch-1](https://github.com/Snap888/WinNative/tree/Snap888-patch-1).

The current WinNative branch includes SnapFrame in both display paths:

- **Embedded X11 renderer:** `app/src/main/cpp/winlator/vk/vk_renderer.c` owns the render/composite/present path. It creates `SnapFrameVk` on the renderer's existing Vulkan device and records compute work into the current frame command buffer. Generated images are routed through the renderer's existing swapchain/present cadence.
- **Wayland compositor:** `app/src/main/cpp/waylandcomp/src/framegen_engine.c` wraps the portable `SnapFrameBackendV1` core and passes a borrowed `SnapFrameVulkanFrame` payload to the WinNative Vulkan backend.

These are two host adapters over the WinNative SnapFrame Vulkan implementation; they are not one shared presenter. X11 currently calls `snap_frame_vk_process/generate` directly, while Wayland uses the portable lifecycle/callback bridge.

### X11 functionality present in WinNative

- SnapFrame is a distinct engine alongside LSFG and DIS/WFG.
- Uses the X11 renderer's existing Vulkan device, queue and command buffer; it does not create a second Vulkan device.
- Uses renderer-owned swapchain images and present synchronization.
- Generates up to three intermediate phases for ×2/×3/×4, subject to swapchain capacity and history availability.
- Includes luma pyramid, bidirectional block-matching flow, consistency/occlusion mask and warp passes.
- Exposes `nativeIsSnapFrameGenerationAvailable`, enable/disable and Debug Flow JNI methods through `VulkanRenderer.java`.
- **Debug Flow** visualizes the forward motion field and consistency/occlusion mask in the live X11 output. The engine still computes forward and backward flow for interpolation.
- Logs delivered real/generated frame counters and per-stage GPU timestamp telemetry.

Key implementation files:

- `app/src/main/cpp/winlator/vk/vk_renderer.c` — embedded X11 renderer integration, capability probe, frame routing, Debug Flow and counters.
- `app/src/main/runtime/display/renderer/VulkanRenderer.java` — Java/JNI controls for X11.
- `app/src/main/cpp/snapframe/src/snap_frame_vk.c` — WinNative Vulkan compute backend and Snap AI policy application.
- `app/src/main/cpp/snapframe/shaders/flow_debug.comp` — flow visualization shader.
- `app/src/main/cpp/waylandcomp/src/framegen_engine.c` — Wayland portable-core adapter.
- `app/src/main/cpp/thirdparty/snapframe` — vendored portable core and Vulkan interop API.
- `app/src/main/cpp/snapai` — vendored Snap AI policy.

### Snap AI status and limitations

Snap AI is instantiated by the Vulkan backend and consumes completed, non-blocking GPU timestamp telemetry. Its current motion magnitude, motion complexity, flow confidence and artifact-risk inputs are neutral fixed priors, not measurements reduced from the actual optical-flow field. The policy therefore adapts primarily to measured GPU pressure; it is not yet content-aware.

Scene discontinuity reset currently uses explicit reset events and a frame gap greater than 500 ms; pixel-content scene-cut detection is not implemented. Do not describe coarse-to-fine optical-flow refinement or content-aware artifact prediction as complete.

### Validation status

The WinNative APK workflow `Tag APK Artifacts` run [42134152224](https://github.com/Snap888/WinNative/actions/runs/42134152224) completed successfully for Antutu, Ludashi, Pubg and Standard on commit `43fdd2f6a3cb214e52bc41134d7e8b9a36f8504e`.

This confirms the four APK build jobs, not device-level frame-generation correctness. Contract/WSI tests and APK builds do not replace physical Turnip/Adreno validation. Still required on device:

1. Verify the X11 Debug Flow view updates continuously during motion.
2. Confirm generated and presented counters increase and match the selected multiplier's phase budget.
3. Check ×2/×3/×4 temporal ordering and FIFO pacing.
4. Test resize, format/HDR changes, pause/resume and device recreation.
5. Run sustained scenes and check for black/stale frames, semaphore errors and regressions.
6. Measure GPU stage times and compare source FPS with delivered FPS.

## Generic Winlator ports

Do not assume that every Winlator-derived application contains the WinNative X11 or Wayland adapters. A generic Winlator port needs a separate host adapter for its exact renderer/compositor and DXVK fork/version.

For D3D9/10/11 games, a DXVK adapter is suitable only when it can access the runtime-owned Vulkan device, command buffer, source image and output slots. For WineD3D or native Vulkan paths, an integration at the compositor/presenter is needed. In all cases the host must retain ownership of device/queue, command submission, WSI semaphores, swapchain and presentation.

The legacy DXVK files in this repository are migration/reference code, not proof of a production-ready adapter for every Winlator fork. Generic Winlator runtime/APK validation remains separate from the confirmed WinNative branch integration.

## Portable source synchronization

WinNative vendors the portable SnapFrame C API and Snap AI policy from the `SnapFrameBackendV1` line. At the time of this update, the corresponding portable headers/core and Snap AI sources have matching Git blob IDs between the two repositories; the WinNative Vulkan implementation remains host-specific and must not be replaced with the legacy DXVK implementation.
