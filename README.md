# Snap AI Engine + SnapFrame

**SnapFrame** is the frame-generation engine in this repository. **Snap AI Engine** is an optional adaptive policy controller that can tune SnapFrame decisions. Both have independently usable portable APIs; neither portable core requires DXVK.

## Branch `SnapFrameBackendV1`

This branch adds a concrete `SnapFrameBackendV1` bridge for the Skyrim Android DXVK fork, while keeping the portable core backend-neutral. The adapter routes DXVK's real-frame callback through the portable engine and delegates GPU work to the imported Vulkan compute pipeline. It preserves the existing midpoint/WSI present hooks and real-frame fallback.

**Scope is important:** this adapter targets the custom DXVK-native 1.9.2b fork in `Snap888/Skyrim_Android` branch `frame_gen`. It is not a drop-in binary for arbitrary upstream DXVK or all Winlator forks. Winlator and WinNative still need their own fork/compositor wiring and runtime validation. See [DXVK adapter status](docs/DXVK_ADAPTER_STATUS.md) and [integration matrix](docs/INTEGRATION_MATRIX.md).

## Architecture

```text
Game / compositor -> SnapFrame portable C ABI -> SnapFrameBackendV1
                                             |-> DXVK Vulkan adapter (Skyrim fork)
                                             |-> WinNative compositor adapter (future)
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

## Integrations

- [Skyrim_Android / DXVK](docs/INTEGRATION_SKYRIM_DXVK.md)
- [Winlator](docs/INTEGRATION_WINLATOR.md)
- [WinNative](docs/INTEGRATION_WINNATIVE.md)
- [Integration workflows](docs/INTEGRATION_WORKFLOWS.md)

Host workflow templates are in `integrations/workflows/`. They must be copied into each host repository and adapted to its actual build system. A successful portable-library build does not mean the host adapter is integrated.

## Preserved source and provenance

Original SnapFrame host sources and four GLSL compute shaders were imported from `Snap888/Skyrim_Android`, branch `frame_gen`, source commit `c385a66daef920f83a86e7eaf16ff822e507e3c7`. The original GPU host is retained under `framegen/backends/dxvk/legacy`; design notes and test reports are under `docs/legacy/`. DXVK keeps its own license; see [NOTICE](docs/NOTICE.md).

## Validation limits

The portable engine and mock-backend tests are covered by CI. The DXVK adapter is wired to the imported Skyrim fork's hooks but has not yet been built in that downstream Android project or tested on a physical GPU. No Winlator or WinNative runtime validation is claimed.
