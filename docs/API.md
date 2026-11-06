# Snap AI policy C API reference

Header: `include/snap_ai/snap_ai.h`. API version: 1.

This document describes the adaptive policy controller. The separate C++ presentation telemetry API is documented in [FRAME_TELEMETRY_API.md](FRAME_TELEMETRY_API.md).

## Lifecycle

1. Call `snap_ai_default_config`.
2. Override profile, GPU budget and allowed flow-scale bounds.
3. Create one handle per renderer context.
4. Submit metrics after previous workload measurements become available.
5. Apply the returned decision to a subsequent safe flow dispatch.
6. Destroy the handle at renderer teardown.

## Input metrics

All normalized values are in [0,1].
- `frame_id`: strictly increasing source-frame identifier.
- `frame_time_ms`: source frame interval (reserved diagnostic in v0.1).
- `gpu_time_ms`: measured duration of the controlled workload.
- `motion_magnitude`: normalized global motion (reserved diagnostic in v0.1).
- `motion_complexity`: host-defined normalized spatial/temporal complexity.
- `flow_confidence`: confidence from matcher/consistency pass.
- `gpu_pressure`: host estimate combining utilization, queue backlog and thermal throttling.
- `artifact_risk`: host estimate from occlusion, forward/backward disagreement or residual.
- `scene_cut`: hard history reset.
- `telemetry_valid`: set only when required metrics are trustworthy.

Never fabricate confidence or GPU pressure. If required telemetry is unavailable, set `telemetry_valid=0`.

## Decisions

- `flow_scale`: multiplier to map to validated discrete host tiers.
- `search_radius_scale`: optional matcher radius multiplier.
- `confidence_threshold`: recommended confidence gate.
- `refinement_passes`: suggested maximum refinement passes.
- `reuse_previous_flow`: advisory only; host must validate history continuity.
- `run_bidirectional_flow`: suggestion for expensive consistency path.
- `run_artifact_check`: suggestion to run host artifact/occlusion checks.
- `estimated_compute_saving`: heuristic estimate, never a measured speedup.

## ABI and errors

All public structs carry `struct_size`; config carries `api_version`. v1 requires the full v1 struct size. Do not change struct layout without an ABI/versioning plan.

Statuses: OK, INVALID_ARGUMENT, VERSION_MISMATCH, INTERNAL. Ignore decision on any error.

## Related APIs

- [Frame presentation telemetry](FRAME_TELEMETRY_API.md) — C++ recorder for host-reported source/generated presentation events. It is separate from the policy C ABI and does not feed the policy controller automatically.
