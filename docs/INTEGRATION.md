# Host integration overview

SnapFrame is a portable engine; host-specific capture and presentation remain in host repositories. Snap AI is an optional scalar-telemetry policy controller.

## General adapter contract

- Create one engine/policy context per renderer as required by the API.
- Feed measured GPU timestamps and validated flow/quality metrics.
- Apply bounded decisions to subsequent frames only.
- Clamp recommendations to device/backend capabilities.
- Keep frame IDs and telemetry validity explicit.
- Preserve the real-frame path and baseline renderer on errors or disable.
- Keep Vulkan resource ownership and synchronization with the host.

## WinNative

Current primary integration target. There are separate embedded X11 VulkanRenderer and Wayland compositor adapters in Snap888/WinNative:Snap888-patch-1. X11 uses a direct backend path; Wayland wraps the portable SnapFrameBackendV1 callback. Both keep WSI ownership in the host. See [WinNative integration](INTEGRATION_WINNATIVE.md) for acquire timeouts, metrics and limitations.

## Skyrim_Android / DXVK

The Skyrim fork has host-specific DXVK integration and legacy DXVK-bound source. Private hooks do not automatically apply to upstream DXVK or another fork. Build and runtime validation belong to the exact Skyrim branch. Preserve existing engine selection/fallback behavior and review third-party licensing before packaging.

## Winlator

A generic Winlator runtime adapter is not complete. Select the exact fork and DXVK/compositor version first, then implement an adapter for its renderer/presentation ownership. A portable ARM64 library build alone does not prove game integration.

## Other hosts

A host may implement SnapFrameBackendV1 or a suitable Vulkan interop adapter. It remains responsible for capture, device/context lifecycle, command submission, output targets, WSI, UI, packaging and actual present-success telemetry.
