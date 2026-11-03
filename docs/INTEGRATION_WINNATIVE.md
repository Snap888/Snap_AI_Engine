# WinNative integration

WinNative has its own compositor and may already contain LSFG/WFG paths. Recommended integration point is the compositor where real frames are available and generated frames can be inserted into the final present sequence. This can cover DXVK and WineD3D without coupling SnapFrame to either API translation layer.

Checklist:
1. Identify compositor frame lifecycle and output surface ownership.
2. Implement a WinNative backend over its existing GPU device and command submission path.
3. Map native handles to opaque `uintptr_t` only at the ABI boundary.
4. Preserve current LSFG/WFG behavior behind an explicit selection/fallback; do not silently replace it.
5. Reuse host timing/pacing and ensure generated frames respect the compositor's frame limiter.
6. Add runtime settings and A/B GPU timing telemetry.

No WinNative adapter is included yet.
