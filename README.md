# Snap AI Engine + SnapFrame

**SnapFrame** is the frame-generation engine in this repository. **Snap AI Engine** is an optional adaptive policy controller that can tune SnapFrame decisions. Both have independently usable portable APIs; neither portable core requires DXVK.

Repository: https://github.com/Snap888/Snap_AI_Engine

## Project status

This branch contains:
- Original SnapFrame host implementation and four GLSL compute shaders imported from `Snap888/Skyrim_Android`, branch `frame_gen`.
- A portable C ABI and dependency-free orchestration core (`framegen/`).
- Mock-backend tests for lifecycle, bypass, hot config, scene cuts, ordering and stats.
- Original DXVK-bound implementation preserved under `framegen/backends/dxvk/legacy/`.
- Original design, Vulkan architecture, readme and device test report preserved under `docs/legacy/`.

**Important:** the portable core and mock backend can be built independently. The imported DXVK host still depends on DXVK internal C++ classes and is preserved as a legacy migration source. It has not yet been fully rewritten against `SnapFrameBackendV1`. Do not interpret portable mock tests as proof of GPU frame generation.

## Architecture

```
Game / compositor / runtime
          | opaque frame handle + metadata
          v
+--------------------------------------+
| SnapFrame Engine (portable C ABI)    |
| lifecycle, config, frame sequencing, |
| bypass/fallback, stats               |
+-------------------+------------------+
                    | SnapFrameBackendV1
          +---------+-----------+
          |                     |
       DXVK                  WinNative
   (first adapter)          (future)
          |
   Vulkan commands, resources,
   barriers, synchronization,
   present scheduling

Optional Snap AI Engine consumes scalar telemetry and returns
bounded policy decisions; it never owns GPU objects or sync.
```

The core treats native handles as opaque tokens. A backend owns graphics objects and synchronization. This allows DXVK, Winlator and WinNative to share the engine without sharing DXVK internals or a Vulkan device.

## Preserved SnapFrame pipeline

- Real-frame capture and history handling in the original DXVK host.
- Half-resolution luma preprocessing.
- Bidirectional block-matching motion fields.
- Forward/backward consistency checks.
- Midpoint warp reconstruction.
- Dedicated log, GPU timing CSV, flow CSV, vector CSV and event CSV.
- Seven-phase diagnostic suite.
- Four compute shaders and shader compilation/validation script.

Imported implementation files are preserved byte-for-byte under `framegen/backends/dxvk/legacy`; original reports are retained. This repository move does not claim new performance or device compatibility.

## Build portable libraries

Requirements: CMake 3.16+, C++17 compiler.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DSNAP_AI_BUILD_TESTS=ON -DSNAP_FRAME_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Build only SnapFrame:
```sh
cmake -S framegen -B build-framegen -DSNAP_FRAME_BUILD_TESTS=ON
cmake --build build-framegen --parallel
ctest --test-dir build-framegen --output-on-failure
```

## Use SnapFrame

Link `SnapFrame::Engine` and include `snap_frame/snap_frame.h`. The host supplies a backend vtable. See [API](docs/FRAMEGEN_API.md), [backend contract](docs/FRAMEGEN_BACKEND.md) and [example](examples/framegen_mock_backend.cpp).

The core supports hot configuration changes when the backend implements `update_config`. Without it, GPU-dependent changes fail explicitly rather than silently pretending to apply.

## Integrations

- [Skyrim_Android / DXVK](docs/INTEGRATION_SKYRIM_DXVK.md)
- [Winlator](docs/INTEGRATION_WINLATOR.md)
- [WinNative](docs/INTEGRATION_WINNATIVE.md)

## Optional DXVK legacy host

The legacy host is an optional integration target requiring a parent DXVK build and its `dxvk_d3d9` target. The portable engine does not need DXVK. See `framegen/backends/dxvk/legacy/CMakeLists.txt` and the migration notes. Existing Skyrim-specific hook points are not automatically injected into arbitrary DXVK forks.

## Documents

- [Architecture](docs/FRAMEGEN_ARCHITECTURE.md)
- [API](docs/FRAMEGEN_API.md)
- [Backend contract](docs/FRAMEGEN_BACKEND.md)
- [Performance and telemetry](docs/FRAMEGEN_PERFORMANCE.md)
- [Testing](docs/FRAMEGEN_TESTING.md)
- [Migration record](docs/FRAMEGEN_MIGRATION.md)
- [Legacy source documentation](docs/legacy/)

## Provenance and licensing

Imported from `Snap888/Skyrim_Android`, branch `frame_gen`, source commit `c385a66daef920f83a86e7eaf16ff822e507e3c7`. DXVK retains its own license; it is not relicensed or included in this repository. See [NOTICE](docs/NOTICE.md).

## Current limitations

- Portable SnapFrame core and mock-backend test are implemented.
- Original GPU implementation remains DXVK-coupled in a clearly marked legacy folder.
- End-to-end DXVK adapter against the new ABI is not yet complete or device-validated.
- No Android APK or real-device test was run as part of this migration.
- Winlator and WinNative adapters are documented targets, not completed adapters.
