# WinNative integration: X11 and Wayland

Target: [Snap888/WinNative](https://github.com/Snap888/WinNative/tree/Snap888-patch-1), branch Snap888-patch-1.

WinNative has two distinct SnapFrame host paths. They share Vulkan compute concepts but not the presenter or all UI/format behavior. Validate them independently.

## X11 — embedded VulkanRenderer

The X11 renderer creates SnapFrame on its existing Vulkan device and records compute into its renderer-owned command path. The renderer owns swapchain acquisition, output/composite targets, queue submission, synchronization and present. SnapFrame does not create a second device/queue or take ownership of WSI semaphores.

Current features:
- up to three generated phases under ×2/×3/×4 scheduling;
- shared flow estimation followed by phase-specific generation;
- GPU timestamps and asynchronous four-value flow reduction;
- Debug Flow modes for motion, matcher confidence and forward/backward consistency;
- bounded generated-slot acquire waits.

**Format limitation:** X11 SnapFrame currently gates output to VK_FORMAT_R8G8B8A8_UNORM. Internal FP16 intermediates do not imply HDR swapchain output support.

## Wayland — compositor adapter

The compositor adapter wraps the portable SnapFrameBackendV1 API and passes borrowed Vulkan handles/output slots. The compositor retains queue submission, WSI semaphore ownership, FIFO scheduling and presentation. Its format/HDR negotiation is separate from X11.

## WSI acquire policy

The goal is to prevent generated-frame acquisition from blocking the host for multiple long serial waits.

- X11: first generated slot uses a refresh-aware timeout bounded to 8–16 ms; subsequent generated slots use 1 ms best-effort waits.
- Wayland: first generated slot waits one refresh interval, clamped to 4–16 ms; unknown refresh defaults to 16 ms. Additional generated slots use 1 ms.

VK_TIMEOUT means acquisition did not complete within the requested interval; it is not by itself device loss. A generated frame computed but not acquired/presented must not be counted as displayed. These timeouts are pacing safeguards, not a closed-loop AI controller.

## Snap AI status

The Vulkan backend supplies GPU timestamp telemetry and asynchronous GPU reductions: normalized mean motion magnitude; magnitude deviation (complexity proxy); forward/backward-consistency-weighted confidence; and confidence/consistency risk proxy.

The reduction uses a bounded multi-slot readback path; it does not require full-frame CPU readback or synchronous GPU waiting. The risk proxy is not calibrated artifact probability and magnitude deviation is not complete spatial/edge-aware complexity.

The next AI improvement is to incorporate actual WSI acquire/present delivery ratio, queue/backlog signals and richer quality metrics. Until then, do not claim Snap AI dynamically controls generation multiplier based on swapchain misses. AI must never control Vulkan synchronization or presentation ownership.

## Validation status and constraints

Portable library tests and host CI are separate from physical-device validation. Check the latest WinNative Actions run before stating an APK build passed. Physical Turnip/Adreno validation remains required for X11 and Wayland independently. Verify no black/stale frames, chronological phase order, acquire/present counts, resize, device loss/recovery, format/HDR and long-run thermal stability. Do not infer X11 format support from Wayland HDR paths or vice versa.
