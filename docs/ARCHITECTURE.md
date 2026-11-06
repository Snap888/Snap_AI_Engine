# Architecture

## Scope

Snap AI Engine is an explainable AI-Lite policy controller, not a frame interpolator or neural image model. It consumes scalar telemetry and returns bounded recommendations for a later frame's flow workload.

The repository also contains a separate passive frame-presentation telemetry recorder. It counts host-reported successful source/generated presentation events and computes bounded rolling interval/CPU-duration statistics. It is not a graphics hook or frame pacer.

## Data flow

```text
Game/API -> DXVK adapter or Vulkan compositor -> GPU/workload telemetry
       -> Snap AI Engine -> SnapAIDecision
       -> SnapFrame core -> Vulkan flow/warp/reconstruction -> host present

Host successful-present boundary -> PresentEvent -> Recorder -> diagnostics/UI
```

The host remains responsible for capture, GPU resources, image processing, Vulkan barriers, semaphores, queue ownership, swapchain lifecycle and present scheduling. The recorder observes host events only; it does not alter this path.

## Controller stages

1. Validate structure sizes and finite telemetry.
2. Bypass on disabled state, invalid metrics, scene cut or non-monotonic frame ID.
3. Smooth motion complexity, flow confidence and GPU pressure using EMA.
4. Estimate scene ease and uncertainty.
5. Select profile baseline.
6. Apply GPU pressure governor.
7. Apply hysteresis and output bounded policy.
8. Update aggregate statistics.

The controller is AI-Lite policy logic. Its estimates are not measurements; hosts must provide genuine metrics and must not fabricate confidence or artifact risk.

## Presentation recorder

- Caller supplies monotonic nanosecond timestamps and a logical presentation ID.
- Events are classified as source or generated and associated with a swapchain generation.
- Duplicate IDs are rejected; stale-generation events are rejected.
- A bounded ring stores recent accepted samples; snapshots calculate median/p95/p99 intervals and CPU present-duration percentiles.
- The recorder does not derive or publish FPS itself and does not establish physical scanout timing.

See [Frame telemetry API](FRAME_TELEMETRY_API.md) for the complete integration contract and metric semantics.

## Threading and memory

Calls on one Snap AI policy handle are serialized by a mutex. Prefer one handle per renderer context. The policy engine uses small CPU state, no per-frame image buffers, no Vulkan objects and no GPU readback.

The telemetry recorder also serializes `record`, `reset` and `snapshot` with a mutex. `record` uses preallocated bounded storage and performs no dynamic allocation or graphics operation, but it is **not lock-free**. `snapshot` allocates temporary CPU vectors and sorts the bounded sample set; call it from a diagnostics/UI thread, not from a latency-sensitive present callback. Do not destroy either object while another thread uses it.

## Failure model

On any non-OK policy status, ignore the decision and use baseline settings. Invalid telemetry events are rejected and counted. A missing optional library or telemetry recorder must never prevent the host renderer from starting.
