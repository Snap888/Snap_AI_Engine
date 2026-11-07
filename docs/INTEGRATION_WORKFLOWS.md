# Host integration workflows

The host repositories own their Android Gradle/CMake projects, SDK pins, driver/runtime revisions and native presentation code. Portable-engine CI and host-runtime CI are separate validation layers.

## Current host workflows

- [Skyrim_Android](https://github.com/Snap888/Skyrim_Android/tree/snapframe-backend-v1): host-specific DXVK adapter compile workflow. A full APK build may additionally require generated gen/ sources or a supplied gen_url archive.
- [WinNative](https://github.com/Snap888/WinNative/tree/Snap888-patch-1): two SnapFrame host paths: direct X11 VulkanRenderer integration and separate Wayland compositor adapter. The backend consumes GPU timestamps and asynchronous flow reductions. WSI acquire/present delivery feedback is not yet a closed-loop Snap AI input. WinNative CI owns APK flavor builds and native contract/WSI tests. Physical Turnip validation is a separate device stage.
- [Winlator dev OSS](https://github.com/Snap888/winlator-dev-oss/tree/snapframe-backend-v1): portable SnapFrame tests and Android arm64 library cross-compile only. A Winlator DXVK/runtime adapter is not yet present.

Workflow templates under integrations/workflows/ are examples; host application workflows must run in the host repositories.

## Next integration steps

### Skyrim_Android

Resolve host DXVK adapter compile errors, build the full APK with generated sources, then test on target Adreno/Mali devices. Validate rapid toggles, resize, scene cuts, device recreation, GPU timings, present results and audio stability.

### WinNative

1. Keep X11 and Wayland adapters separate while sharing the portable engine and Vulkan compute implementation where practical.
2. Verify X11 output-format gate and Vulkan format capability checks against formats actually used by shaders.
3. Test X11 Debug Flow and confirm GPU stage timing and generated-frame counters.
4. Instrument actual acquire attempts/successes/timeouts/errors and successful generated presents in both X11 and Wayland.
5. Correlate WSI events to source frame, phase/slot and swapchain generation.
6. Feed validated rolling WSI delivery ratio into Snap AI; lower multiplier/slot budget on repeated misses and restore gradually after stable delivery.
7. Poll presentation-recorder snapshots from a diagnostics/UI thread and compare them with existing host counters and captured output.
8. Run physical Turnip/Adreno tests for X11 and Wayland independently, including resize, device loss, enable/disable and sustained FIFO pacing.
9. Expand flow metrics with edge-aware complexity, occlusion ratio, photometric residual and search-boundary saturation.

### Winlator

1. Select and pin the exact Winlator fork and DXVK revision.
2. Port host hooks and adapter to that fork's private interfaces; do not copy Skyrim's Android launcher/runtime.
3. Preserve host Vulkan/WSI ownership and generated-frame pacing.
4. Add runtime controls, per-game config, Debug Flow and source/generated frame counters.
5. Add fork-specific build CI and physical Adreno/Mali validation.

## CI security and validation

- Pin third-party actions to reviewed major versions or immutable SHAs according to host policy.
- Do not store game files, APK signing keys or personal device logs in public artifacts.
- Upload only build outputs and sanitized test logs.
- Keep hardware/device validation as a separate self-hosted or manual stage.

Do not mark an integration hardware-validated based only on portable tests, cross-compilation or APK compilation.
