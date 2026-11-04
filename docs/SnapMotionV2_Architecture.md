# SnapMotionV2 architecture — Stage 1

Status: portable metadata/configuration core implemented; V2 Luma, pyramid search, refinement, validation, static-mask and interpolation shaders are source-level and CI compile-validated. DXVK resources/dispatch and runtime integration are not yet connected.

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


## Shader implementation progress — 2026-11-04

The separate `framegen/backends/dxvk/shaders/motion_v2/` namespace now contains:
- L0 luma and L0->L1 downsample;
- coarse/fine block search with temporal/spatial seed support and explicit border rejection;
- 3x3 median + subpixel refinement and local confidence;
- independent forward/backward round-trip validation;
- temporal static-region mask (not a semantic HUD classifier);
- bidirectional time-parametric interpolation with confidence fallback, gamut clamp and optional real-frame detail recovery.

All V2 shader sources are included in the shader compiler and validated as Vulkan 1.1 SPIR-V in CI run [#41810539818](https://github.com/Snap888/Snap_AI_Engine/actions/runs/41810539818). The portable core unit tests, SnapFrame build and standalone SnapFrame tests also passed in that run.

This is shader compilation validation only. It does not prove descriptor compatibility with DXVK, image format support on Adreno, synchronization correctness, visual quality, performance, or Skyrim runtime behavior. No V2 shader is currently dispatched by the game.


## Async GPU telemetry reduction

Two compute passes are now present in the V2 shader namespace:
- workgroup partial reduction (8x8 cells per group) computes motion magnitude,
  normalized local vector variation, mean confidence, low-confidence ratio,
  occlusion ratio, static ratio and same-position luma delta;
- final reduction produces one 80-byte record tagged with source frame ID,
  timestamp and extent. It marks the record invalid when less than 90% of the
  expected flow cells contributed valid samples.

The static mask is sampled in normalized flow UV so a full-resolution mask can
be reduced against the lower-resolution flow grid. The final record is designed
for an asynchronous small-buffer readback; full flow-vector readback and GPU
waits are not part of the design.

Artifact risk and scene-cut are provisional heuristics. They must be calibrated
against real scenes and must not drive Snap AI until DXVK integration proves
that each record's frame ID and history epoch match the source frame.
