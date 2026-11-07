# Candidate reconstruction policy for SnapFrame

This note specifies the AI-facing contract for the experimental four-candidate Vulkan
reconstruction implemented by the WinNative SnapFrame host.

## Inputs already available to the controller

SnapFrame produces forward and backward block-match flow, matcher confidence, photometric
match cost, and a forward/backward consistency mask. These signals are sufficient for a first
candidate path; no external neural weights are required.

## Candidate generation

At each active flow cell, construct four displacement alternatives:

1. Previous-frame sample using the primary forward estimate.
2. Current-frame sample using the primary backward estimate.
3. Previous-frame sample using the best confidence-weighted, edge-compatible cardinal neighbour.
4. Current-frame sample using the corresponding backward neighbour estimate.

Candidate metadata stores the UV displacement, confidence, and in-bounds validity. The
displacement must be applied relative to each full-resolution output pixel; using the flow-cell
center as an absolute UV would produce visible 8x8 block tiles.

## Candidate scoring

The scorer compares the forward/backward photometric pair for each primary/alternative pair.
Candidate logits combine log confidence and photometric disagreement, then use a numerically
bounded softmax. Invalid coordinates receive zero weight. If every candidate is unreliable,
the weight vector is zero and reconstruction must use the real current frame as a sharp fallback.

This is an intentionally interpretable baseline. It is not a trained neural network and does not
claim semantic occlusion understanding.

## AI control contract

The host exposes three modes:

- AUTO: Snap AI enables the candidate path for SNAP_AI_MODE_QUALITY below 0.75 GPU pressure, disables above 0.90, and retains the previous state between thresholds.
- OFF: force the original midpoint warp.
- ON: force candidate reconstruction for A/B testing.

The AI should eventually decide using both artifact risk and incremental GPU cost. The first
integration uses quality mode as a conservative gate because the candidate stage adds compute.
The next policy iteration should add independent candidate-stage timing, fallback ratio,
candidate entropy, and occlusion ratio to frame telemetry before making finer decisions.

## Dataset labels to collect

For paired real/generated sequences, retain:

- source and generated frame identifiers and phase;
- forward/backward flow and consistency;
- candidate coordinates and normalized weights;
- candidate pair photometric error;
- per-pixel fallback mask;
- GPU time for warp, score, and reconstruction separately;
- human or objective labels for ghosting, disocclusion, blur, and edge tearing.

Do not treat the archive's embedded FP16 constants as reusable model weights: provenance,
license, training set, tensor contract, and intended inference precision are unknown.
