# Snap AI Engine + SnapFrame

**SnapFrame** is the frame-generation engine in this repository. **Snap AI Engine** is an optional adaptive policy controller that can tune SnapFrame decisions. Both have independently usable portable APIs; neither portable core requires DXVK.

## Branch `SnapFrameBackendV1`

This branch adds a concrete `SnapFrameBackendV1` bridge for the Skyrim Android DXVK fork, preserves the original GPU pipeline and shaders, and adds host CI branches/workflows for Skyrim, Winlator and WinNative.

**Compatibility boundary:** the Skyrim adapter targets the custom dxvk-native 1.9.2b fork in `Snap888/Skyrim_Android` branch `frame_gen`. Winlator and WinNative have separate native integration points and currently receive portable SnapFrame CI only; their runtime adapters are not implemented yet. See [DXVK adapter status](docs/DXVK_ADAPTER_STATUS.md) and [integration matrix](docs/INTEGRATION_MATRIX.md).

## Architecture

```text
Game / compositor -> SnapFrame portable C ABI -> SnapFrameBackendV1
                                             |-> DXVK Vulkan adapter (Skyrim fork)
                                             |-> Winlator DXVK adapter (port pending)
                                             |-> WinNative compositor adapter (port pending)
Optional Snap AI consumes scalar telemetry and returns bounded policy settings.
```

The core treats native handles as opaque tokens. A backend owns graphics objects, command recording, synchronization and generated-frame insertion. DXVK is one backend, not where the engine's policy or public API lives.

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
- [WinNative](https://github.com/Snap888/WinNative/tree/snapframe-backend-v1) — portable engine tests and Android arm64 library workflow; compositor adapter pending.

Detailed guides: [Skyrim](docs/INTEGRATION_SKYRIM_DXVK.md), [Winlator](docs/INTEGRATION_WINLATOR.md), [WinNative](docs/INTEGRATION_WINNATIVE.md), [workflow status](docs/INTEGRATION_WORKFLOWS.md).

## Preserved source and provenance

Original SnapFrame host sources and four GLSL compute shaders were imported from `Snap888/Skyrim_Android`, branch `frame_gen`, source commit `c385a66daef920f83a86e7eaf16ff822e507e3c7`. The original GPU host is retained under `framegen/backends/dxvk/legacy`; design notes and test reports are under `docs/legacy/`. DXVK keeps its own license; see [NOTICE](docs/NOTICE.md).

## Validation limits

Portable core tests and Linux/Windows/Android arm64 builds pass in CI. The Skyrim DXVK adapter compile workflow is configured separately against the exact fork; no physical GPU or APK/device validation is claimed yet. Winlator and WinNative currently build the portable SnapFrame library only, not a working runtime frame-generation adapter.
