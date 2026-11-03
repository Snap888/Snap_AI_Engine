# Winlator integration

Winlator can use SnapFrame through a DXVK backend when the selected runtime owns the Vulkan device and present chain through DXVK. Prefer DXVK for D3D9/10/11 games; a compositor adapter is preferable when generation must cover WineD3D or native Vulkan paths.

Checklist:
1. Select a Winlator DXVK fork/version and confirm compatible Vulkan queue/present behavior.
2. Port only SnapFrame hook points and adapter code, not Skyrim's Android launcher/runtime.
3. Keep frame resources and synchronization owned by the selected runtime.
4. Implement `SnapFrameBackendV1`; pass host handles as opaque values only.
5. Keep generated present on the same pacing/synchronization path as real frames.
6. Provide runtime toggles, profile settings, per-game config and diagnostics.
7. Validate Adreno and Mali independently.

No Winlator adapter is included yet.
