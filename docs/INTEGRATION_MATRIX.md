# Integration matrix

| Host | Intended adapter | Build target | Status |
|---|---|---|---|
| Skyrim_Android | DXVK Vulkan adapter | Android arm64-v8a + APK | `SnapFrameBackendV1` bridge wired to imported Skyrim DXVK hooks; downstream build/device test pending |
| Winlator (DXVK runtime) | DXVK adapter rebuilt against selected Winlator fork | Android arm64-v8a runtime | Porting guide and workflow template; not integrated or runtime-validated |
| Winlator (WineD3D/native Vulkan paths) | Host compositor adapter | Android arm64-v8a | Separate adapter required |
| WinNative | Native compositor/backend adapter | Windows x86_64 / ARM64 as supported by host | Porting guide and workflow template; not implemented |
| Other DXVK ports | DXVK adapter rebuilt against that fork | Host-specific | Requires versioned hook compatibility review |

A successful Snap_AI_Engine build proves only that the portable engine compiles. A successful DXVK adapter compile proves private API compatibility but not correct GPU synchronization or present pacing. Every host needs its own runtime and device validation.
