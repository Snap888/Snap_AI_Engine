# Snap AI Engine + SnapFrame

**SnapFrame** is the frame-generation engine in this repository. **Snap AI Engine** is its optional adaptive policy controller. Both expose portable APIs and do not require DXVK.

## Branch `SnapFrameBackendV1`

This branch provides the portable C ABI, multiplier validation through ×4, compute shaders and a host-owned Vulkan interop contract in `framegen/include/snap_frame/snap_frame_vulkan.h`.

**Compatibility boundary:** the Skyrim adapter targets the custom dxvk-native 1.9.2b fork in `Snap888/Skyrim_Android` branch `frame_gen`. Winlator and WinNative have separate host adapters.

## Architecture

```text
Game / compositor -> SnapFrame portable C ABI -> SnapFrameBackendV1
                                             |-> Skyrim DXVK adapter
                                             |-> Winlator DXVK adapter (runtime port pending)
                                             |-> WinNative Wayland Vulkan adapter (experimental)
Optional Snap AI consumes bounded telemetry and returns adaptive policy settings.
```

The portable core treats native handles as opaque. Vulkan adapters borrow the host device, queue, command buffer, source image and output slots. The host retains queue submission, swapchain and present-semaphore ownership.

## Build and test

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

Requirements: CMake 3.16+ and a C++17 compiler. See [API](docs/FRAMEGEN_API.md), [backend contract](docs/FRAMEGEN_BACKEND.md), [developer guide](docs/README_BACKEND_V1.md), and [testing](docs/FRAMEGEN_TESTING.md).

## Integrations and CI branches

- [Skyrim_Android / DXVK](https://github.com/Snap888/Skyrim_Android/tree/snapframe-backend-v1) — native CMake integration and DXVK adapter compile workflow.
- [Winlator dev OSS](https://github.com/Snap888/winlator-dev-oss/tree/snapframe-backend-v1) — portable engine tests and Android arm64 library workflow; runtime adapter pending.
- [WinNative](https://github.com/Snap888/WinNative/tree/Snap888-patch-1) — experimental Wayland compositor Vulkan adapter, portable-core callback bridge, Snap AI GPU-timing policy and game/overlay settings. CI and physical Turnip validation are pending.

Detailed guides: [Skyrim](docs/INTEGRATION_SKRYRIM_DXVK.md), [Winlator](docs/INTEGRATION_WINLATOR.md), [WinNative](docs/INTEGRATION_WINNATIVE.md), [workflow status](docs/INTEGRATION_WORKFLOWS.md).

## Portable multiplier contract

The core accepts multiplier 1 (bypass) and 2..4 (up to three generated temporal phases). It validates that a backend never returns more than `multiplier - 1` frames. A host must provide distinct chronological phases and enough output slots; the API does not fabricate missing frames.

## Validation limits

The WinNative branch now has a concrete runtime path, but this repository does not claim a successful physical Turnip run. Scene discontinuity resets are currently driven by explicit core events and a >500 ms frame gap; pixel-based scene-cut classification and measured optical-flow confidence/motion-complexity inputs remain future work. WinNative's contract tests validate phase timing, flow-grid bounds, resize/format/scene-cut/device-generation reset flags and absence of backend-owned WSI operations.
