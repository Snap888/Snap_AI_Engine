# Winlator integration

Targeted CI branch: [`Snap888/winlator-dev-oss:snapframe-backend-v1`](https://github.com/Snap888/winlator-dev-oss/tree/snapframe-backend-v1). Its workflow runs portable SnapFrame tests and cross-compiles the Android arm64 library. This validates the dependency/API, not frame generation in a running Winlator session.

Winlator can use SnapFrame through a DXVK backend when the selected runtime owns the Vulkan device and present chain through DXVK. Prefer DXVK for D3D9/10/11 games; a compositor adapter is preferable when generation must cover WineD3D or native Vulkan paths.

Checklist for the runtime adapter:
1. Select the exact Winlator port and DXVK fork/version used by its runtime. The user has not specified a preferred fork; `winlator-dev-oss` is the current CI baseline, not a claim that every Winlator port uses it.
2. Port the SnapFrame hook points and adapter code to that fork; do not copy Skyrim's Android launcher/runtime.
3. Keep frame resources and synchronization owned by the selected runtime.
4. Implement `SnapFrameBackendV1`; pass host handles as opaque values only.
5. Keep generated present on the same pacing/synchronization path as real frames.
6. Provide runtime toggles, profile settings, per-game config and diagnostics.
7. Validate Adreno and Mali independently.

No Winlator runtime adapter or APK-level frame-generation test is included yet.
