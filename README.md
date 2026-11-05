# Snap AI Engine + SnapFrame

**SnapFrame** is the frame-generation engine in this repository. **Snap AI Engine** is an optional adaptive policy controller that can tune SnapFrame decisions. Both have independently usable portable APIs; neither portable core requires DXVK.

## Branch `SnapFrameBackendV1`

This branch provides the portable C ABI, multiplier validation through ×4, imported SnapFrame compute shaders, and a host-owned Vulkan interop contract. The Vulkan contract is in `framegen/include/snap_frame/snap_frame_vulkan.h`.

**Compatibility boundary:** the Skyrim adapter targets the custom dxvk-native 1.9.2b fork in `Snap888/Skyrim_Android` branch `frame_gen`. Winlator and WinNative have separate native integration points; the Vulkan ABI does not itself constitute a working host adapter.

## Architecture

```text
Game / compositor -> SnapFrame portable C ABI -> SnapFrameBackendV1
                                             |-> DXVK Vulkan adapter (Skyrim fork)
                                             |-> Winlator DXVK adapter (port pending)
                                             |-> WinNative compositor adapter (port pending)
Optional Snap AI consumes scalar telemetry and returns bounded policy settings.
```

The core treats native handles as opaque tokens. A Vulkan adapter may use the separate Vulkan interop payload, while the host retains device, queue, command-buffer, swapchain and present-semaphore ownership. DXVK is one backend, not where the engine's policy or public API lives.

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
- [WinNative](https://github.com/Snap888/WinNative/tree/Snap888-patch-1) — Vulkan interop contract documented; native compositor adapter and GPU pipeline remain pending.

Detailed guides: [Skyrim](docs/INTEGRATION_SKYRIM_DXVK.md), [Winlator](docs/INTEGRATION_WINLATOR.md), [WinNative](docs/INTEGRATION_WINNATIVE.md), [workflow status](docs/INTEGRATION_WORKFLOWS.md).

## Portable multiplier contract

The core accepts multiplier 1 (bypass) and 2..4 (up to three generated temporal phases). It validates that a backend never returns more than `multiplier - 1` frames. This does not mean every adapter implements ×3/×4; actual generation requires distinct GPU reconstruction phases and host-side presentation support.

## Validation limits

Portable core CI covers Linux/Windows/Android arm64 builds. This repository's Vulkan interop header defines ownership and frame-resource rules, but no WinNative runtime adapter or physical-GPU validation is claimed.
