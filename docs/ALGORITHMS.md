# Algorithms and adaptive control

## SnapFrame image-generation pipeline

SnapFrame is the pixel-generation component; Snap AI is not an interpolator. The current Vulkan pipeline includes:
1. Luma extraction/downsample and multi-level pyramid preparation.
2. Forward and backward block/local motion estimation.
3. Motion-field fusion.
4. Forward/backward consistency mask.
5. GPU-side scalar reductions for AI diagnostics.
6. Per-phase candidate warping, scoring/reconstruction and output composition.

The key compute optimization is to estimate motion once for a consecutive source-frame pair and reuse that field for every generated temporal phase. Increasing multiplier should add phase-specific synthesis, not repeat the complete flow solve. Cover this invariant with dispatch counters.

## Snap AI-Lite policy

The current controller is deterministic and explainable, not a trained neural network:
- smooth measured complexity, confidence and GPU pressure;
- score scene simplicity versus motion uncertainty and risk;
- select a profile baseline;
- limit workload under GPU pressure;
- apply hysteresis to prevent rapid tier oscillation;
- bypass on invalid metrics, disabled state, scene discontinuity or invalid frame ordering.

Current WinNative GPU reductions provide mean normalized motion magnitude, magnitude deviation (complexity proxy), consistency-weighted confidence and a confidence/consistency risk proxy. GPU timestamps provide workload timing. Magnitude deviation is only a complexity proxy; risk is not calibrated artifact probability.

## AI next steps: flow intelligence

Priority features:
- robust p50/p95 flow magnitude and vector-direction variance;
- edge-aware spatial flow disagreement;
- forward/backward residual distribution and occlusion/disocclusion ratio;
- photometric warp residual where a valid reference exists;
- search-boundary saturation;
- scene-cut score and source-frame continuity;
- measured per-stage GPU cost, queue backlog and thermal state where available;
- WSI acquire attempts/successes/timeouts and successful generated presents.

All features need normalization, frame IDs, validity flags and asynchronous delivery. Never block the render path for CPU readback. AI decisions apply to a subsequent frame and are clamped to host-supported tiers.

## WSI-aware policy — planned

A future controller should combine smoothed acquire-success ratio with GPU budget and quality risk:
- reduce requested generation slots/multiplier when acquisition misses rise;
- restore quality gradually after a stable window (hysteresis);
- never request more generated slots than recent delivery budget supports;
- keep source-frame presentation as guaranteed fallback;
- share policy semantics across X11 and Wayland while preserving each host's WSI ownership.

This is not yet implemented as a closed-loop policy. Current bounded X11/Wayland acquisition timeouts are host-side safeguards, not proof of AI-driven WSI adaptation.

## Research directions

- Global-motion pre-warp to compensate coherent camera movement before local flow.
- Robust flow regularization with independently switchable outlier rejection and edge-preserving smoothing.
- Occlusion-aware synthesis and confidence-weighted source selection.
- Candidate reconstruction and scoring using confidence and consistency.
- Diagnostic views for motion, confidence, occlusion, candidates and blend weights, with no extra work when disabled.
- Optional tiny learned residual only after classical flow is measured and ground-truth data exists; classical SnapFrame remains default and fallback.

Do not train a full neural interpolator as the next step. A future compact model must be optional, quantized, bounded by a measured inference budget, validated on held-out scenes and disabled/fallback-safe on unsupported hardware.
