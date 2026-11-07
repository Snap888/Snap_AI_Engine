# Flow diagnostics, visualization and training-data contract

## Purpose

A flow viewer is a debugging/calibration tool, not a second frame-generation path. Snap AI consumes compact numeric reductions, not rendered debug images.

## Current WinNative diagnostics

The active Vulkan backend has a selectable X11 debug output for:
- motion direction/magnitude;
- block-matcher confidence;
- forward/backward consistency.

These modes reuse the existing debug compute pass and push-constant selection; they must not create a second presenter or alter normal frame scheduling. The Wayland adapter has its own host path; do not assume X11 UI/debug controls are automatically available there.

Future views: occlusion/disocclusion, forward/backward error, warped candidates, blend weights and offline final residual. Debug disabled must add no dispatches or allocations to the normal path.

## Snap AI feature contract

WinNative reduces the existing flow/consistency images into four normalized values:
1. mean normalized flow magnitude;
2. magnitude deviation (complexity proxy);
3. matcher confidence weighted by forward/backward consistency;
4. forward-confidence inconsistency risk proxy.

The 1x1 RGBA32F result is copied to a small host-visible staging slot in a bounded ring. CPU mapping occurs only after the associated timestamp-query slot is complete. This avoids full-image CPU readback and explicit GPU waits.

Important semantics:
- magnitude deviation is not a complete edge-aware spatial-vector complexity measure;
- the fourth value is a confidence/consistency conflict proxy, not a calibrated artifact probability or pure FB residual;
- GPU timestamp budget pressure is not hardware utilization or thermal state;
- unavailable values must be marked invalid, never replaced by fabricated zeroes/perfect confidence.

Next signals: edge-aware vector disagreement, explicit occlusion ratio, photometric warp residual, search-boundary saturation, scene-cut score and queue/thermal metrics where available.

## WSI delivery telemetry — next integration

Current X11 and Wayland host code uses bounded generated-slot acquire waits. That is a pacing safeguard, not yet a closed-loop AI feature.

Instrument actual acquire and successful present boundaries separately:
- acquire attempts/successes/timeouts/errors;
- generated present success/failure;
- source frame, phase/slot and swapchain generation;
- monotonic event timestamp.

Do not count compute completion or acquire success as a displayed frame. Feed only validated rolling delivery statistics to a future Snap AI policy. The policy may recommend a lower multiplier/slot budget after repeated misses and restore slowly after stable delivery; host code retains all WSI synchronization and presentation ownership.

## Ground-truth capture

Use real high-frame-rate source footage with a true middle frame as target. Store triplets (I0, I_mid_GT, I1), source timestamps, dimensions, frame IDs, build/driver metadata, consent status and hashes. Capture must be opt-in and locally stored by default. Use a bounded worker queue; never block the render thread for readback, compression or disk I/O.

Never use a generated SnapFrame or GSFG frame as ground truth. Split datasets by game/scene/session to prevent adjacent-frame leakage.

## Evaluation

Measure PSNR/SSIM with correct color-space handling, robust photometric error, edge/gradient error around silhouettes, temporal consistency and occlusion-boundary error. Confidence calibration must be monotonic against held-out ground-truth error.

A neural residual model is a later optional experiment only. Classical flow/warp/synthesis remains the default and fallback.
