# Architecture

## Scope
Snap AI Engine v0.1 is an explainable AI-Lite policy controller, not a frame interpolator or neural image model. It receives scalar telemetry and returns bounded recommendations for the next frame's flow workload.

## Data flow
```text
Game/API -> DXVK adapter or Vulkan compositor -> scalar telemetry
       -> Snap AI Engine -> SnapAIDecision
       -> Snap Framegen Core -> Vulkan flow/warp/reconstruction -> Present
```

The host remains responsible for capture, GPU resources, image processing, Vulkan barriers, semaphores, queue ownership and present scheduling.

## Controller stages
1. Validate structure sizes and finite telemetry.
2. Bypass on disabled state, invalid metrics, scene cut or non-monotonic frame ID.
3. Smooth motion complexity, flow confidence and GPU pressure using EMA.
4. Estimate scene ease and uncertainty.
5. Select profile baseline.
6. Apply GPU pressure governor.
7. Apply hysteresis and output bounded policy.
8. Update aggregate statistics.

## Threading and memory
Calls on one handle are serialized by a mutex. Prefer one handle per renderer context. The engine uses small CPU state, no per-frame image buffers, no Vulkan objects and no GPU readback. Do not destroy a handle while another thread uses it.

## Failure model
On any non-OK status, ignore the decision and use baseline settings. A missing library must never prevent the host renderer from starting.
