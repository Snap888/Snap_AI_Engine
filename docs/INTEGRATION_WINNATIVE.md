# WinNative integration

Target branch: [`Snap888/WinNative:Snap888-patch-1`](https://github.com/Snap888/WinNative/tree/Snap888-patch-1).

**WinNative revision checked:** `f200bbcd43e7cdf02513f9e9820d1eb7ef541381` (2026-11-07).

WinNative has two distinct host paths for SnapFrame. The embedded X11 Vulkan renderer has a direct SnapFrame Vulkan backend integration. The embedded Wayland compositor has a separate adapter using the portable `SnapFrameBackendV1` C API. Do not describe this as a single shared presenter or reuse the Skyrim DXVK adapter.

## Embedded X11 path — integrated, device validation pending

The X11 path is implemented in `app/src/main/cpp/winlator/vk/vk_renderer.c`:

- `create_snap()` creates `SnapFrameVk` with the renderer's existing `VkDevice` and `VkPhysicalDevice`.
- Frame processing is recorded into the renderer's existing frame command buffer.
- The renderer owns swapchain acquisition, composite targets, frame scheduling, submission and presentation. SnapFrame does not create a second device/queue or take ownership of WSI semaphores.
- The renderer plans up to `multiplier - 1` generated frames (maximum three), calls `snap_frame_vk_process()`, then records each temporal phase with `snap_frame_vk_generate()` into renderer-owned composite targets.
- Generated images are copied through the existing X11 swapchain path.
- `nativeSetSnapFrameGenerationEnabled` and `nativeSetSnapFrameDebugFlow` expose runtime controls.
- The in-game X11 frame-generation pane exposes SnapFrame, multiplier, Flow Scale and Debug Flow controls.

Current X11 limitation: `vk_renderer.c` explicitly gates SnapFrame to `VK_FORMAT_R8G8B8A8_UNORM` swapchains. Do not claim general HDR/RGBA16F swapchain support for this host path. The backend's internal RGBA16F flow images do not remove that output-format restriction.

## Embedded Wayland path — separate adapter

The Wayland compositor path uses:

- `app/src/main/cpp/thirdparty/snapframe`: portable SnapFrame C API and Vulkan interop ABI.
- `app/src/main/cpp/snapai`: vendored Snap AI policy and frame telemetry modules.
- `app/src/main/cpp/snapframe/src/snap_frame_vk.c`: Vulkan compute backend.
- `app/src/main/cpp/waylandcomp/src/framegen_engine.c`: `SnapFrameBackendV1` callback adapter.
- `framegen_bridge.c`, `vk_present.c` and `effects_chain.c`: output ring and compositor FIFO present integration.

The compositor borrows its existing Vulkan device, command buffer and output slots. It retains queue submission, WSI semaphore ownership and presentation.

## Snap AI behavior and current limitation

The WinNative Vulkan backend creates the AI controller and feeds GPU timestamp telemetry. The current X11/Wayland policy inputs still include neutral fixed priors for motion magnitude, motion complexity, flow confidence and artifact risk rather than measured GPU-reduced image/flow signals. Consequently, adaptive behavior is presently driven primarily by measured GPU pressure and bounded/hysteresis rules. Do not describe this as content-aware optical-flow adaptation.

The decision fields for search radius, refinement passes, flow reuse and artifact checks are not all wired to distinct Vulkan shader controls yet.

## Frame presentation recorder — compiled, not live-wired

WinNative's `app/src/main/cpp/snapai` now contains:

- `include/snap_ai/frame_telemetry.h`
- `src/frame_telemetry.cpp`

The `snap_ai` static CMake target compiles the recorder alongside the policy controller and exports its public API. This makes the recorder available to native adapters, but **does not connect it to any presenter yet**.

Required next work:
1. Instrument only successful source/generated presents at the outermost X11 Vulkan presentation boundary and the Wayland compositor's actual present-completion boundary.
2. Assign unique logical IDs to displayed events; share an ID only for nested/wrapped callbacks for the same presentation.
3. Increment swapchain generation on recreation and reset recorder history.
4. Keep snapshot polling off the latency-sensitive present path.
5. Compare recorder output with existing host counters and device capture before exposing it as live FPS or feeding it into AI.

Do not add a new Present hook, queue submission, pacing loop or frame-generation pipeline. The host retains all synchronization and display ownership. See [Frame telemetry API](FRAME_TELEMETRY_API.md).

## WinNative build and runtime validation

The WinNative repository owns the Android Gradle/CMake build. Its CI should be used for the APK flavor matrix and native contract/WSI tests. A passing portable Snap_AI_Engine build is not a substitute for that host build.

Physical Turnip/Adreno validation remains required for both X11 and Wayland paths. For X11, record the device/driver, output format, multiplier, source FPS, real/generated counters, stage timings, Debug Flow output, resize behavior and sustained pacing. Confirm no black/stale frames, correct chronological phase order, clean enable/disable and device-loss recovery before declaring runtime support validated.

Known limitations:
- X11 SnapFrame output currently requires RGBA8 UNORM.
- Scene-cut reset in the Vulkan backend is frame-gap/event based; pixel-content scene-cut detection is not implemented.
- AI motion/confidence/artifact inputs are neutral priors, not measured flow reductions.
- Frame telemetry is compiled but not connected to presenter call sites.
- Physical device validation is not implied by CI or static ownership tests.
