# WinNative integration

Target branch: [`Snap888/WinNative:Snap888-patch-1`](https://github.com/Snap888/WinNative/tree/Snap888-patch-1).

WinNative now contains an experimental Wayland compositor adapter using the SnapFrame portable C API and a native Vulkan backend. This is a separate host adapter from Skyrim's DXVK integration.

## Runtime path

- `app/src/main/cpp/thirdparty/snapframe`: vendored portable SnapFrame C ABI and Vulkan host interop header.
- `app/src/main/cpp/snapai`: vendored Snap AI policy controller.
- `app/src/main/cpp/snapframe/src/snap_frame_vk.c`: Vulkan compute implementation and timestamp-query telemetry.
- `app/src/main/cpp/waylandcomp/src/framegen_engine.c`: creates `SnapFrameEngine`, registers `SnapFrameBackendV1` callbacks and constructs the per-frame Vulkan payload.
- `framegen_bridge.c`: provides the ring output slots and routes accepted generated frames into the existing FIFO present path.
- `vk_present.c` and `effects_chain.c`: provide sampled views for scene and post-effect source images. The compositor retains all queue submission and present semaphore ownership.

## GPU pipeline

1. Downsample luma to half, quarter and eighth resolution.
2. Estimate forward and backward flow.
3. Run flow consistency and produce an occlusion mask.
4. Reconstruct distinct temporal phases at `1/(M+1)`, `2/(M+1)`, ... for multiplier M.
5. Write only to compositor-owned ring slots; the existing FIFO loop submits and presents them.

Flow Scale changes adjust the active flow grid through push constants; the maximum flow images remain allocated, avoiding per-frame resource recreation.

## Snap AI

Vulkan timestamp queries measure five stage groups: luma, pyramid, flow, consistency and warp. Completed query slots are collected non-blockingly several frames later. Snap AI uses the measured total GPU time against a 5 ms budget and returns a bounded adaptive ratio; the ratio is multiplied by the user's Flow Scale baseline, so AI can reduce work under pressure but cannot silently exceed the selected baseline.

The current AI metrics use neutral motion/confidence priors. A GPU reduction for measured flow confidence, motion complexity and artifact risk remains future work.

## Ownership and pacing

The adapter borrows the compositor's device, queue, command buffer, source view and output slots. It never creates a second device/queue, submits the command buffer, waits for GPU completion, performs CPU readback, owns semaphores or calls present. The compositor's per-frame fence boundary makes resize-time resource replacement safe. Generated frames are passed to the existing FIFO loop.

## Settings

- Per-game Game Settings: choose LSFG or SnapFrame, enable generation, select multiplier and set the Flow Scale baseline.
- In-game overlay: select OFF/LSFG/DIS/SnapFrame, choose multiplier and adjust Flow Scale.
- SnapFrame appears as available only after the Wayland compositor reports the required storage/sample formats and timestamp-query support on the current Vulkan device.

## Validation and remaining work

The WinNative branch includes standalone contract tests for phase fractions, Flow Scale grid bounds, RGBA8/RGBA16F, resize, format change, scene-cut reset, arm toggle and device-generation change. A static test rejects backend-owned queue submissions, present calls, semaphore creation and GPU waits.

The branch CI runs the APK flavor matrix and these tests. A physical Turnip run is still required. Current scene-cut reset is event/frame-gap based (>500 ms), not image-content detection. The next improvement is GPU-derived flow-confidence/motion-complexity telemetry and a real-device stress run for pacing, black/stale frames and ×2/×3/×4 ordering.
