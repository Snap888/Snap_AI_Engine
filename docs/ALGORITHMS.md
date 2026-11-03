# Algorithms and future neural backend

## AI-Lite v0.1
The first release is deterministic adaptive control, not a trained neural network:
- EMA smoothing for complexity, confidence and pressure.
- Easy-scene score rewards simple motion with high confidence and low artifact risk.
- Difficulty/uncertainty score increases work when motion is complex or confidence is poor.
- Pressure governor limits requested work when GPU pressure is high.
- Hysteresis limits frame-to-frame flow-scale oscillation.
- Scene cuts, invalid telemetry and repeated IDs force bypass.

## Why start without a neural network?
A model may cost more GPU time than it saves, requires representative training data and can fail outside its training distribution. Establish a deterministic baseline and trustworthy telemetry first.

## Future model seam
A later optional model may map a fixed feature vector to complexity, confidence, artifact risk and a recommended tier. Requirements:
- documented feature schema and normalization;
- finite, bounded outputs and confidence threshold;
- quantized compact model;
- no dynamic allocation in render hot path;
- measured inference budget;
- heuristic fallback on unsupported hardware or low confidence;
- model optional and disabled by default.

Possible runtimes to evaluate only after benchmarking: LiteRT/TFLite, ONNX Runtime Mobile or custom Vulkan compute. NPU use is optional, not a dependency.

## Safety
Never allow AI to control Vulkan synchronization/resource ownership. Do not reuse flow across scene cuts or incompatible history. Reuse is advisory and host-validated. Invalid telemetry means bypass.
