# WinNative integration

Targeted CI branch: [`Snap888/WinNative:snapframe-backend-v1`](https://github.com/Snap888/WinNative/tree/snapframe-backend-v1). Its workflow runs portable SnapFrame tests and cross-compiles the Android arm64 library. It does not yet link SnapFrame into the WinNative compositor.

WinNative has its own Wayland compositor frame-generation path. Relevant current integration points are `app/src/main/cpp/waylandcomp/src/framegen_bridge.c`, `framegen_engine.c`, and `vk_present.c`; the existing engine path also compiles LSFG/DIS sources from `app/src/main/cpp/winlator/vk`. This is not the Skyrim DXVK hook ABI.

Recommended SnapFrame backend integration:
1. Extend the compositor's framegen engine selection with an explicit SnapFrame option; do not silently replace LSFG/DIS.
2. Implement a Vulkan backend over the compositor's existing `VkDevice`, `VkQueue` and `VkCommandBuffer` lifecycle.
3. Call the portable `snap_frame_process` from the scene-frame hook after effects and before the final present sequence.
4. Make generated images visible to the compositor's existing transfer/blit path and preserve its current pacing.
5. Map JNI/UI controls to bounded hot configuration changes; apply resource recreation only at the compositor thread's safe boundary.
6. Keep existing LSFG/DIS paths and their tests unchanged; add SnapFrame-specific GPU timing, fallback, resize and device-loss tests.

No WinNative SnapFrame Vulkan backend or runtime APK test is included yet.
