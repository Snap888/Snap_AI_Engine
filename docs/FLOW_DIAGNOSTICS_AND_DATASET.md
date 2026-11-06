# Flow diagnostics, visualization and training-data contract

## Goals

A flow viewer is a debugging/calibration tool, not a second frame-generation path. It should make the intermediate decisions visible so that artifacts can be traced to motion estimation, occlusion handling, warping or compositing. Snap AI should consume compact numeric reductions, not rendered debug images.

## Diagnostic views

**Current WinNative X11 status:** the active Vulkan backend already has `flow_debug.comp`, a debug pipeline and `snap_frame_vk_debug_into()`. The renderer calls it when `snap_debug_flow` is enabled and presents the result as a full-frame HSV flow + consistency view. This is already a useful first flow viewer; it is not a separate overlay and it is not wired into Snap AI. The inspected Wayland adapter does not currently expose this debug mode. Preserve this working path rather than creating a second viewer/presenter.

The next visualization increment is to add selectable confidence, occlusion, forward/backward error, warp-candidate and blend-weight views. GPU-side scalar reductions for Snap AI have now been added to WinNative `Snap888-patch-1`; keep extending the existing viewer rather than adding another presenter. These modes must use the host's output/presentation path and must not alter normal frame scheduling.

| View | Contents | Diagnoses |
|---|---|---|
| Flow HSV | Direction mapped to hue, magnitude to value/intensity; show scale and saturation legend | Wrong direction, search saturation, discontinuities |
| Flow confidence | Confidence heatmap | Regions where matcher is uncertain |
| Forward/backward error | Per-pixel consistency residual | Occlusion, disocclusion and mismatched vectors |
| Occlusion mask | Valid/invalid or graded visibility | Ghosting and holes at object boundaries |
| Warp A / Warp B | Each source warped independently to selected phase | Which source contributes invalid pixels |
| Blend weights | Per-source contribution weights | Double exposure / cross-dissolve |
| Photometric residual | Difference between warped candidates and target/reference where available | Bad flow vs lighting/exposure change |
| Final residual | Generated frame compared with true middle frame in offline evaluation | Actual interpolation error |

Provide a selectable phase for 3x/4x output and a freeze-frame/step mode in the diagnostics UI. Do not force GPU readback for every frame. Prefer rendering a debug image into a host-provided slot; if CPU export is enabled, use a bounded asynchronous staging/readback queue and drop samples rather than blocking rendering.

## Snap AI feature contract

The policy controller currently accepts normalized scalar metrics through `SnapAIFrameMetrics`. In WinNative `Snap888-patch-1`, the X11 Vulkan backend now reduces the already-computed forward flow and consistency images into four normalized metrics. A 256-lane compute workgroup calculates mean normalized flow magnitude, standard deviation of magnitude, mean matcher confidence weighted by forward/backward consistency, and complementary artifact risk. The 1x1 RGBA32F result is copied into a 16-byte host-visible/coherent staging slot in a four-slot ring; CPU mapping occurs only after the associated timestamp-query slot is available. This avoids full-image readback and explicit GPU waits. Snap AI uses the measured motion magnitude and complexity, confidence, artifact risk and timestamp-derived GPU pressure. If the required Vulkan buffer functions, coherent host memory or RGBA32F storage image are unavailable, content telemetry is skipped and SnapFrame interpolation remains available. Wayland still needs a matching telemetry handoff; this implementation does not imply that Wayland already feeds these metrics to AI. Current metric limitations: complexity is magnitude deviation rather than a full spatial-vector/edge-aware measure; artifact risk is confidence/consistency-derived and does not yet include photometric residuals; GPU pressure is timestamp-budget pressure, not hardware utilization. Populate those fields from real GPU reductions/timestamps:

| AI feature | Source | Use |
|---|---|---|
| motion_magnitude | robust flow magnitude reduction | Global motion load |
| motion_complexity | spatial variance / edge-aware flow disagreement | Search/refinement budget |
| flow_confidence | confidence/FB-consistency reduction | Decide whether to trust reuse and whether to run expensive checks |
| artifact_risk | occlusion ratio + photometric residual + boundary saturation | Enable fallback/refinement |
| gpu_time_ms | non-blocking timestamp results for the controlled workload | GPU budget governor |
| gpu_pressure | measured utilization/queue backlog/thermal state where available | Performance cap |
| scene_cut | image/luma divergence or host scene-cut detector | Reset temporal history |

All features need explicit validity and stable normalization. Do not treat unavailable values as real zeroes or perfect confidence. Preserve the safe bypass behavior on invalid telemetry.

## Training and calibration data

### Ground truth

Best data is a real high-frame-rate capture: take frames at native high rate, then construct input endpoints around a skipped true middle frame. The true middle frame is the target. This supports both objective calibration and optional model training.

A pair of consecutive 60 FPS frames alone does not contain the true 30 FPS midpoint. A midpoint synthesized by SnapFrame is not ground truth; using it as the target would teach a model to reproduce existing errors.

### Capture rules

- Capture real source frames before SnapFrame interpolation, not generated frames.
- Prefer lossless image data (PNG/QOI or equivalent) and preserve original timestamps, dimensions, format and source-frame IDs.
- Store triplets `(I0, I_mid_GT, I1)`, plus optional SnapFrame output and diagnostic scalar features for comparison.
- Add a manifest with session ID, frame IDs, timestamps, resolution, game/build/driver identifiers, capture mode, consent status, and hashes.
- Capture must be opt-in, clearly disclosed, locally stored by default and never silently uploaded. Do not include account identifiers, personal paths or other PII in the manifest.
- Use a bounded queue and dedicated worker for encoding/writing. If overloaded, count dropped captures; never stall the render thread.
- Record the exact engine/backend commit and settings for every session.

### Labels and metrics

For each triplet and phase, store objective measures where a true target exists:
- PSNR / SSIM (with known color-space handling);
- Charbonnier or robust L1 photometric error;
- edge/gradient error around silhouettes;
- temporal consistency and occlusion-boundary error;
- optional perceptual metric for offline evaluation only.

Calibrate confidence thresholds against error distributions. A confidence value is useful only if high-confidence regions are measurably more accurate than low-confidence regions.

### Dataset split and release

Split by game/scene/session, not random adjacent frames, to prevent near-duplicate leakage between train and validation. Keep test scenes held out. Public benchmark datasets may have non-commercial or unclear media rights; do not use them for releasable weights without a documented license review. Final release weights should use self-captured footage for which capture and use rights have been checked.

## Model strategy

Do not train a full neural interpolator as the next step. The previous Adreno measurements recorded in project history showed the full-frame CNN path far above the real-time budget and had Vulkan operator limitations. First improve and measure classical flow/warp/synthesis. If a model is later justified, evaluate a small optional residual correction at reduced resolution, without neural warping or a mandatory runtime dependency. Classical SnapFrame must remain the fallback.

## Acceptance gates

1. Debug views visually match the underlying flow/confidence/occlusion buffers on known synthetic patterns.
2. Debug disabled has no additional GPU dispatches or allocations on the normal path.
3. GPU reductions are non-blocking and their frame IDs match the source pair they describe.
4. Confidence calibration is monotonic against held-out ground-truth error.
5. Dataset manifest validates every blob/hash and rejects missing consent metadata where required by capture policy.
6. Model inference (if later added) is optional, quantized, bounded by a measured GPU budget and never controls Vulkan synchronization.
