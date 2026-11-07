# Snap AI policy C API

Header: include/snap_ai/snap_ai.h. API version: 1.

This API is the deterministic adaptive policy controller. It does not generate pixels and is separate from the passive C++ presentation telemetry recorder.

## Lifecycle

1. Call snap_ai_default_config and set profile, budget and bounds.
2. Create one handle per renderer context.
3. Submit completed, trustworthy measurements with a monotonically increasing source-frame ID.
4. Apply a valid decision to a later safe flow dispatch, never retroactively to the frame already measured.
5. Destroy the handle only after the host has stopped using it.

## Metrics

Normalized values are in [0,1] unless otherwise stated:
- frame_id: strictly increasing source-frame identifier.
- frame_time_ms: source frame interval.
- gpu_time_ms: measured controlled-workload GPU duration in milliseconds.
- motion_magnitude: normalized motion magnitude.
- motion_complexity: normalized host complexity estimate.
- flow_confidence: normalized matcher/consistency confidence.
- gpu_pressure: normalized host pressure estimate; timestamp budget alone is not hardware utilization.
- artifact_risk: normalized risk proxy based on available occlusion/consistency/residual signals.
- scene_cut: host-detected discontinuity/history reset.
- telemetry_valid: true only when required inputs are trustworthy.

WinNative currently supplies asynchronous GPU reductions for motion magnitude, magnitude deviation, consistency-weighted confidence and a confidence/consistency risk proxy, plus GPU timestamp measurements. The proxy is not calibrated ground truth. Additional edge-aware complexity, occlusion ratio, photometric residual and boundary-saturation measurements remain planned.

Never fabricate confidence, pressure or artifact risk. If required telemetry is missing, mark telemetry invalid and use safe bypass behavior.

## Decisions

- flow_scale: map to validated discrete host tiers and clamp to supported dimensions.
- search_radius_scale: advisory matcher-workload recommendation.
- confidence_threshold: quality gate.
- refinement_passes: suggested maximum refinement work.
- reuse_previous_flow: advisory only; host validates source-pair continuity.
- run_bidirectional_flow: advisory only; host may retain mandatory consistency requirements.
- run_artifact_check: advisory quality check.
- estimated_compute_saving: heuristic estimate, never measured speedup.

Not every decision field is connected to a distinct Vulkan control in every host. Check host integration status before assuming a field changes runtime behavior.

## ABI and safety

Public structs carry struct_size; config carries api_version. Do not change struct layout without a versioning plan. Ignore decisions on any non-OK status. AI must never control Vulkan barriers, queue ownership, semaphore waits, swapchain acquisition or present scheduling.

Related: [Frame telemetry API](FRAME_TELEMETRY_API.md), [algorithms](ALGORITHMS.md), [backend ownership](FRAMEGEN_BACKEND.md).
