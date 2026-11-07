# Snap AI Engine + SnapFrame

SnapFrame is the frame-generation engine. Snap AI Engine is its optional adaptive controller and telemetry layer. The project uses a portable C++17 core and host-owned graphics backends; DXVK is not a dependency of the core.

**Strategic objective: surpass GSFG** in measured image quality, successful generated-frame delivery, latency, GPU cost, pacing, power/thermal efficiency, robustness and platform coverage. This is a target, not a current performance claim. Every comparison must use reproducible A/B captures on equivalent hardware and settings.

## Current implementation

- Portable SnapFrame lifecycle/configuration/frame-order core and versioned C ABI.
- Host-owned Vulkan interop contract: backend records compute work into host command buffers and does not own queues, swapchains or WSI semaphores.
- WinNative Snap888-patch-1 has separate embedded X11 VulkanRenderer and Wayland compositor paths.
- GPU pipeline includes luma/pyramid preparation, bidirectional motion estimation, motion fusion, forward/backward consistency, candidate warp/score/reconstruction and phase-specific synthesis.
- Up to three generated temporal phases under ×2/×3/×4 scheduling. Flow is reused across phases of the same source pair.
- Optional deterministic Snap AI policy with GPU timestamps, asynchronous GPU flow reductions, bounded recommendations and hysteresis.
- X11 Debug Flow modes for motion, matcher confidence and forward/backward consistency.
- Bounded generated-slot WSI acquisition waits in both X11 and Wayland.

See [WinNative integration](docs/INTEGRATION_WINNATIVE.md) for platform-specific behavior and limitations.

## Architecture

~~~text
Game / host renderer
  ├─ WinNative X11 VulkanRenderer ───────┐
  ├─ WinNative Wayland compositor ───────┼─> SnapFrame Vulkan compute
  ├─ Skyrim/DXVK host adapter ──────────┤       └─ flow once, synthesize phases
  └─ future Winlator adapter ───────────┘
                                             ↑
GPU timestamps + flow reductions + WSI telemetry (when wired)
                    └──────── Snap AI policy ───── bounded next-frame settings

Host owns device, queues, command submission, swapchain, WSI semaphores and present.
SnapFrame owns private compute resources and temporal processing state.
Snap AI owns CPU policy state; it never owns images or graphics synchronization.
~~~

## AI status

The current controller is AI-Lite, not a trained neural network. It uses deterministic bounded policy logic, smoothing, GPU-budget pressure and hysteresis. WinNative has GPU-side reductions for normalized flow magnitude, magnitude deviation, consistency-weighted confidence and a confidence/consistency risk proxy. These are useful measured signals, but magnitude deviation is not full edge-aware complexity and the risk proxy is not calibrated artifact probability.

The controller bypasses safely when telemetry is invalid. It must not fabricate missing values or control Vulkan synchronization. A neural model is a later optional experiment only if it demonstrates net benefit on target hardware.

## Build and test

Requirements: CMake 3.16+ and a C++17 compiler.

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSNAP_AI_BUILD_TESTS=ON -DSNAP_FRAME_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
~~~

Standalone SnapFrame:

~~~sh
cmake -S framegen -B build-framegen -DSNAP_FRAME_BUILD_TESTS=ON
cmake --build build-framegen --parallel
ctest --test-dir build-framegen --output-on-failure
~~~

Portable CI proves compilation and deterministic tests, not GPU image quality, WSI delivery, scanout timing or physical-device performance. APK builds and device validation belong to host workflows.

## Documentation

- [Architecture](docs/ARCHITECTURE.md)
- [Snap AI API](docs/API.md)
- [Algorithms and AI policy](docs/ALGORITHMS.md)
- [SnapFrame architecture](docs/FRAMEGEN_ARCHITECTURE.md)
- [SnapFrame API](docs/FRAMEGEN_API.md)
- [Vulkan backend ownership](docs/FRAMEGEN_BACKEND.md)
- [Performance and telemetry](docs/FRAMEGEN_PERFORMANCE.md)
- [Testing](docs/FRAMEGEN_TESTING.md)
- [Logging](docs/LOGGING.md)
- [WinNative X11/Wayland integration](docs/INTEGRATION_WINNATIVE.md)
- [Win-FG research audit](docs/WIN_FG_TECHNICAL_AUDIT.md)
- [Flow diagnostics and dataset](docs/FLOW_DIAGNOSTICS_AND_DATASET.md)
- [GSFG comparison plan](docs/GSFG_COMPETITIVE_PLAN.md)
- [Roadmap](docs/ROADMAP.md)

## Integration status

- WinNative: X11 and Wayland SnapFrame host paths are integrated in Snap888-patch-1; see the integration document. Check the latest host CI and device logs separately.
- Skyrim_Android/DXVK: host-specific integration is maintained in that repository. Legacy DXVK-coupled code here is migration/reference material, not a universal backend.
- Winlator: generic runtime adapter is not complete; portable library compilation does not prove in-game integration.
- Physical device: Turnip/Adreno runtime validation remains a separate release gate.

## Safety and ownership

The host owns capture, Vulkan objects, command buffers, queue submission, WSI semaphores, swapchain lifecycle and presentation. SnapFrame records bounded compute work using host-provided resources. Snap AI returns bounded policy recommendations only. On failure, preserve the real-frame path and baseline renderer.

## License

MIT for original code in this repository. Third-party code and shader provenance remain subject to their own notices; see [NOTICE](docs/NOTICE.md).
