# Snap AI Engine + SnapFrame

**SnapFrame** is the frame-generation engine in this repository. **Snap AI Engine** is its optional adaptive policy controller. The portable modules are C++17 and do not require DXVK or a particular host graphics API.

## Branch `SnapFrameBackendV1`

This branch provides the portable Snap AI policy API, frame-presentation telemetry recorder, SnapFrame frame-generation core, compute shader sources and host-owned Vulkan interop contract.

**Compatibility boundary:** the Skyrim adapter targets the custom DXVK fork in `Snap888/Skyrim_Android`. WinNative and Winlator use separate host integration paths. Always verify the exact host branch and CI independently.

## Architecture

```text
Game / host renderer
  ├─ Skyrim: host-specific DXVK adapter
  ├─ WinNative X11: embedded VulkanRenderer -> SnapFrameVk
  ├─ WinNative Wayland: compositor -> SnapFrameBackendV1 -> SnapFrameVk
  └─ Winlator: portable engine; runtime adapter pending

Host presentation events -> optional Snap AI telemetry recorder
GPU flow reductions -> Snap AI policy controller -> bounded recommendations
```

The portable frame-generation core borrows host-owned device, command buffer, source image and output slots. The host retains queue submission, swapchain, WSI semaphores and presentation ownership. Snap AI does not generate or interpolate images and never owns graphics synchronization.

The presentation recorder is a passive diagnostics primitive. It does not hook Present, pace frames, intercept Vulkan/DXGI, perform GPU readback or create another frame-generation pipeline.

## Win-FG research and integration decision

The sibling [Snap888/win-fg](https://github.com/Snap888/win-fg) repository was audited. Its most valuable ideas are flow/synthesis separation (solve motion once per real source pair), global-motion pre-warp, robust flow regularization, occlusion-aware synthesis, flow visualization and opt-in lossless capture. We will reuse compatible algorithms and diagnostic concepts inside the existing SnapFrame backend—not run a second FG pipeline or import opaque SPIR-V.

- [Win-FG technical audit and prioritized port plan](docs/WIN_FG_TECHNICAL_AUDIT.md)
- [Flow diagnostics, AI feature and training-data contract](docs/FLOW_DIAGNOSTICS_AND_DATASET.md)

The reviewed Win-FG revision is MIT-licensed; FSR3-derived shader adaptations require retaining AMD FidelityFX attribution. Verify each file's provenance before importing code. The audit does not claim that the proposed backend changes or flow viewer are already implemented.

## Integration status

- [Skyrim_Android / DXVK](https://github.com/Snap888/Skyrim_Android/tree/snapframe-backend-v1) — host-specific DXVK adapter work; check its compile workflow and device status separately.
- [WinNative](https://github.com/Snap888/WinNative/tree/Snap888-patch-1) — X11 SnapFrame path is implemented in the embedded VulkanRenderer; a separate Wayland adapter uses the portable callback API. The vendored `snap_ai` target includes the passive presentation recorder. **The recorder is not yet connected to real X11/Wayland present-completion events.** Physical Turnip/Adreno validation remains pending.
- [Winlator dev OSS](https://github.com/Snap888/winlator-dev-oss/tree/snapframe-backend-v1) — portable engine tests and Android arm64 library workflow only; runtime DXVK/compositor adapter is not yet implemented.

Detailed guides: [Skyrim](docs/INTEGRATION_SKYRIM_DXVK.md), [Winlator](docs/INTEGRATION_WINLATOR.md), [WinNative](docs/INTEGRATION_WINNATIVE.md), [integration matrix](docs/INTEGRATION_MATRIX.md), [workflow status](docs/INTEGRATION_WORKFLOWS.md).

## Build and test

Requirements: CMake 3.16+ and a C++17 compiler.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Standalone SnapFrame:

```sh
cmake -S framegen -B build-framegen
cmake --build build-framegen --parallel
ctest --test-dir build-framegen --output-on-failure
```

The test suite includes Snap AI policy tests and deterministic frame-telemetry tests. CI also validates native Linux/Windows builds, Android arm64 cross-compilation, shader compilation and standalone SnapFrame.

See [Snap AI API](docs/API.md), [frame telemetry API](docs/FRAME_TELEMETRY_API.md), [SnapFrame API](docs/FRAMEGEN_API.md), [backend contract](docs/FRAMEGEN_BACKEND.md), [developer guide](docs/README_BACKEND_V1.md), and [testing](docs/FRAMEGEN_TESTING.md).

## Portable multiplier contract

The core accepts multiplier 1 (bypass) and 2..4 (up to three generated temporal phases). It validates that a backend never returns more than `multiplier - 1` frames. A host must provide distinct chronological phases and enough output slots; the API does not fabricate missing frames. The planned flow/synthesis split should reuse one flow solution across those phases.

## Validation limits

Portable CI proves compilation and deterministic unit tests, not physical-device image quality, GPU synchronization, presentation delivery or scanout timing. Host adapter builds and device validation must be tracked separately.

WinNative X11 currently requires RGBA8 UNORM output. Its AI integration has measured GPU timing inputs but still uses neutral fixed motion/confidence/artifact priors. The presentation recorder is compiled into the WinNative Snap AI target, but real presenter wiring remains pending. Winlator runtime integration remains pending.
