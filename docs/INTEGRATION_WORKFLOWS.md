# Host integration workflows

The host repositories own their Android Gradle/CMake projects, SDK pins, driver/runtime revisions and native presentation code. Portable-engine CI and host-runtime CI are separate validation layers.

## Current host workflows

- [Skyrim_Android](https://github.com/Snap888/Skyrim_Android/tree/snapframe-backend-v1): host-specific DXVK adapter compile workflow. A full APK build may additionally require generated `gen/` sources or a supplied `gen_url` archive.
- [WinNative](https://github.com/Snap888/WinNative/tree/Snap888-patch-1): contains two SnapFrame host paths: direct X11 `VulkanRenderer` integration and a separate Wayland compositor adapter. Its own CI is responsible for APK flavor builds and native contract/WSI tests. Physical Turnip validation is still a manual/device stage.
- [Winlator dev OSS](https://github.com/Snap888/winlator-dev-oss/tree/snapframe-backend-v1): portable SnapFrame tests and Android arm64 library cross-compile only. A Winlator DXVK/runtime adapter is not yet present.

Workflow templates under `integrations/workflows/` are examples; host application workflows must run in the host repositories.

## Next integration steps

### Skyrim_Android

Resolve any host DXVK adapter compile errors, build the full APK with generated sources, then test on Adreno and Mali. Validate rapid toggles, resize, scene cuts, device recreation, LSFG mutual exclusion, GPU timings, present results and audio stability.

### WinNative

1. Keep X11 and Wayland adapters separate while sharing the portable engine and Vulkan compute implementation where practical.
2. Verify the X11 output-format gate and Vulkan format capability checks against the formats actually used by the shaders.
3. Test X11 Debug Flow and confirm both the GPU stage timing line and increasing generated-frame counter.
4. Verify per-game settings persistence and native state propagation.
5. Run physical Turnip/Adreno tests for X11 and Wayland independently, including resize, device loss, enable/disable and sustained FIFO pacing.
6. Replace neutral Snap AI motion/confidence/artifact priors with measured GPU reductions before claiming content-aware adaptation.

### Winlator

1. Select and pin the exact Winlator fork and DXVK revision used by the target APK.
2. Port the host hooks and adapter to that fork's private interfaces; do not copy Skyrim's Android launcher/runtime.
3. Preserve host Vulkan/WSI ownership and generated-frame pacing.
4. Add runtime controls, per-game config, Debug Flow and source/generated frame counters.
5. Add fork-specific build CI and physical Adreno/Mali validation.

## CI security and validation

- Pin third-party actions to reviewed major versions or immutable SHAs according to host policy.
- Do not store game files, APK signing keys or personal device logs in public artifacts.
- Upload only build outputs and sanitized test logs.
- Keep hardware/device validation as a separate self-hosted or manual test stage.

Do not mark an integration hardware-validated based only on portable tests, cross-compilation or APK compilation.
