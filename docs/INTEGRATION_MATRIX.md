# Integration matrix

| Host | Branch | Intended adapter | CI currently validates | Runtime adapter status |
|---|---|---|---|---|
| Skyrim_Android | `snapframe-backend-v1` | DXVK Vulkan adapter | Exact fork CMake configure/build target `snap_frame_dxvk_legacy` | Adapter wired to imported DXVK hooks; compile/device validation pending |
| Winlator dev OSS (`Snap888/winlator-dev-oss`) | `snapframe-backend-v1` | DXVK adapter rebuilt against selected Winlator fork | Portable core tests + Android arm64 library | Not integrated; fork-specific DXVK hooks still required |
| WinNative (`Snap888/WinNative`) | `snapframe-backend-v1` | Native Wayland compositor/Vulkan adapter | Portable core tests + Android arm64 library | Not integrated; use compositor scene-frame hook, not Skyrim DXVK ABI |
| Other DXVK ports | Host-specific | DXVK adapter rebuilt against that fork | No host-specific CI yet | Requires versioned hook compatibility review |

A successful Snap_AI_Engine build proves only that the portable engine compiles. A successful Skyrim DXVK adapter compile proves private API compatibility but not correct GPU synchronization or present pacing. Every host needs its own runtime and device validation.
