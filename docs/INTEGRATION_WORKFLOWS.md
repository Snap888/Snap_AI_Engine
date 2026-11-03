# Host integration workflows

Three downstream branches now carry CI workflows:

- [Skyrim_Android `snapframe-backend-v1`](https://github.com/Snap888/Skyrim_Android/tree/snapframe-backend-v1/.github/workflows): configures and compiles `snap_frame_dxvk_legacy` against the exact Skyrim DXVK source. A separate manual job can build the full APK when generated `gen/` sources or a `gen_url` archive are supplied.
- [Winlator dev OSS `snapframe-backend-v1`](https://github.com/Snap888/winlator-dev-oss/tree/snapframe-backend-v1/.github/workflows): runs portable SnapFrame core tests and cross-compiles the Android arm64 library. It does not yet validate a Winlator DXVK hook adapter.
- [WinNative `snapframe-backend-v1`](https://github.com/Snap888/WinNative/tree/snapframe-backend-v1/.github/workflows): runs portable SnapFrame core tests and cross-compiles the Android arm64 library. It does not yet validate a compositor adapter.

The workflow templates are also kept in `integrations/workflows/`. Host application workflows must run in the host repositories because only those repositories own their Gradle/CMake projects, SDK pins, signing configuration and DXVK/compositor revisions.

## Next integration steps

### Skyrim_Android

Fix any compile errors in the DXVK adapter workflow, then run the full APK workflow with generated `gen/` sources. Test the adapter on Adreno and Mali devices, including rapid toggles, resize, scene cuts, device recreation, LSFG mutual exclusion, GPU timings, present results and audio stability.

### Winlator

Select the exact Winlator fork and DXVK revision used by the runtime. Port the DXVK copy/generation/present hooks to that fork and build the adapter against its private headers. A passing portable-library job is not proof of this integration.

### WinNative

Implement a compositor backend around the existing Wayland `vkp_framegen_run` / `fge_process` lifecycle and the compositor's own Vulkan device/queue. Preserve existing LSFG/DIS selection and pacing; do not reuse the Skyrim DXVK adapter directly.

## CI security

- Pin third-party actions to reviewed major versions or immutable SHAs according to host policy.
- Do not store game files, APK signing keys or personal device logs in public artifacts.
- Upload only build outputs and sanitized test logs.
- Keep hardware/device validation as a separate self-hosted or manual test stage.
