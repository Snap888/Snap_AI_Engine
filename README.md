# Snap AI Engine + SnapFrame

**SnapFrame** is the frame-generation engine in this repository. **Snap AI Engine** is its optional adaptive policy controller. Both expose portable APIs and do not require DXVK.

## Branch `SnapFrameBackendV1`

This branch provides the portable C ABI, multiplier validation through ×4, compute shaders and a host-owned Vulkan interop contract in `framegen/include/snap_frame/snap_frame_vulkan.h`.

**Compatibility boundary:** the Skyrim adapter targets the custom dxvk-native 1.9.2b fork in `Snap888/Skyrim_Android` branch `frame_gen`. WinNative and Winlator use separate host integration paths.

## Architecture

```text
Game / host renderer
  ├─ Skyrim: DXVK adapter (host-specific)
  ├─ WinNative X11: native VulkanRenderer -> SnapFrameVk
  ├─ WinNative Wayland: compositor -> SnapFrameBackendV1 -> SnapFrameVk
  └─ Winlator: portable engine available; runtime DXVK/compositor adapter pending

Optional Snap AI consumes backend telemetry and returns bounded policy recommendations.
```

The portable core treats native handles as opaque. Vulkan adapters borrow the host device, command buffer, source image and output slots. The host retains queue submission, swapchain, present semaphores and presentation ownership.

## Integration status

- [Skyrim_Android / DXVK](https://github.com/Snap888/Skyrim_Android/tree/snapframe-backend-v1) — host-specific DXVK adapter work; check its compile workflow and device status separately.
- [WinNative](https://github.com/Snap888/WinNative/tree/Snap888-patch-1) — X11 SnapFrame path is implemented directly in the embedded `VulkanRenderer`; a separate Wayland adapter uses the portable callback API. X11 exposes multiplier, Flow Scale and Debug Flow. Physical Turnip/Adreno validation remains pending.
- [Winlator dev OSS](https://github.com/Snap888/winlator-dev-oss/tree/snapframe-backend-v1) — portable engine tests and Android arm64 library workflow only; runtime DXVK/compositor adapter is not yet implemented.

Detailed guides: [Skyrim](docs/INTEGRATION_SKYRIM_DXVK.md), [Winlator](docs/INTEGRATION_WINLATOR.md), [WinNative](docs/INTEGRATION_WINNATIVE.md), [integration matrix](docs/INTEGRATION_MATRIX.md), [workflow status](docs/INTEGRATION_WORKFLOWS.md).

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

## Portable multiplier contract

The core accepts multiplier 1 (bypass) and 2..4 (up to three generated temporal phases). It validates that a backend never returns more than `multiplier - 1` frames. A host must provide distinct chronological phases and enough output slots; the API does not fabricate missing frames.

## Validation limits

WinNative's source integration is ahead of this portable repository's previous documentation. This is not a claim of physical runtime success: verify generated-frame delivery, no black/stale frames, resize and device-loss recovery, and correct ×2/×3/×4 ordering on the target device.

WinNative's current AI adapter feeds measured GPU timings but neutral motion/confidence/artifact priors. WinNative X11 output currently requires RGBA8 UNORM. Winlator runtime integration remains pending.
