# SnapMotionV2 architecture — Stage 1

Status: portable metadata/configuration core implemented; GPU algorithms and DXVK integration are not yet connected.

## Ownership boundaries

- SnapFrame Core remains responsible for frame scheduling, multiplier policy and backend callbacks.
- SnapFrame DXVK Adapter remains responsible for Vulkan/DXVK resources, queue submission, barriers, history image lifetime and Present integration.
- SnapMotionV2 is backend-neutral. Its public API contains configuration and frame-tagged telemetry only; it does not include Vulkan headers or own GPU resources.
- The host must not route Swapchain, Present, game time, audio or Android Surface control into SnapMotionV2.

## Stage 1 API

Public header: `framegen/include/snap_frame/motion_v2.h`.

The module currently provides:
- Safe defaults (disabled, Balanced, optional processing features enabled, adaptive quality disabled).
- Strict config validation.
- Frame-tagged metrics with source frame ID, timestamp and extent.
- Monotonic publication: duplicate/out-of-order and late pre-reset readbacks are rejected.
- Bounded-age retrieval: callers provide current frame ID and maximum acceptable lag.
- Explicit reset reasons for resolution/format/swapchain/frame-gap/engine-switch/device-loss.
- No Vulkan/DXVK dependency and no GPU wait/readback.

Metrics schema: motion magnitude (pixels), motion complexity, mean confidence, low-confidence ratio, occlusion ratio, static ratio, artifact risk, scene-cut and validity. All ratios are normalized to [0,1]; magnitude is finite and non-negative. Metrics are accepted only when telemetry_valid=1.

## CronyFG source audit

The supplied archive contains:
- `cronyfg_engine.{h,cpp}`: Vulkan engine wrapper, quality selection, multiplier/pacer policy, warm-up and ring ownership assumptions.
- `cronyfg_chain.{h,cpp}`: Vulkan resource allocation, descriptor sets, pipelines, barriers and per-pass dispatch.
- `cronyfg_quality.h`: six presets coupling flow resolution and confidence/round-trip/gamut/sharpen tuning.
- Five compute shaders: luma, downsample, search, refine and interpolation.

Transfer decisions:
| CronyFG component | SnapMotionV2 decision |
| --- | --- |
| Luma/downsample | Reuse algorithm after format/layout adaptation |
| Coarse-to-fine search | Port search math, but profile against Snap's measured flow cost |
| Temporal seed | Port only with frame-ID/history-epoch validation |
| Refine/median/subpixel | Port as an isolated pass |
| Forward/backward validation | Port with explicit direction and extent contracts |
| Gamut clamp | Port as an optional interpolation stage |
| Sharpen | Keep opt-in and disabled by default |
| Vulkan Chain/Engine | Do not port; duplicates SnapFrame/DXVK ownership |
| Frame Pacer/passive governor | Do not port; retain SnapFrame's pacing policy |

The archive's engine assumes RGBA8 composite input and a ring size at least multiplier + 2. Those are CronyFG-specific assumptions, not requirements to copy into SnapFrame. SnapFrame's actual image formats, two-history-image model and command submission ownership must be audited in the Skyrim adapter before any shader is connected.

## Next gates

1. Add pure algorithm/reference tests for flow field statistics and validation.
2. Port luma/downsample/search shaders into a separate V2 shader namespace, without replacing V1.
3. Add DXVK-owned persistent resources and async scalar reduction; never read back full vectors for AI.
4. Validate frame IDs, history resets, image layouts and two-frames-in-flight resource ownership.
5. Keep V2 disabled and select V1 until device image-quality/performance tests pass.
