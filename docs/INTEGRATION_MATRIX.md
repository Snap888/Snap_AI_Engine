# Integration matrix

| Host | Intended adapter | Build target | Status |
|---|---|---|---|
| Skyrim_Android | DXVK Vulkan adapter | Android arm64-v8a + APK | Legacy GPU host imported; new ABI adapter not complete |
| Winlator (DXVK runtime) | Same DXVK adapter, built against selected Winlator DXVK fork | Android arm64-v8a runtime | Integration guide and workflow template; not runtime-validated |
| Winlator (WineD3D/native Vulkan paths) | Host compositor adapter | Android arm64-v8a | Separate adapter required |
| WinNative | Native compositor/backend adapter | Windows x86_64 / ARM64 as supported by host | Integration guide and workflow template; not implemented |
| Other DXVK ports | DXVK adapter rebuilt against that fork | Host-specific | Requires versioned hook compatibility review |

A successful Snap_AI_Engine build proves only that the portable engine compiles. It does not prove that a host has the correct frame capture hook, output insertion point, synchronization, shader compatibility or present pacing. Every host adapter needs its own build and runtime validation.
