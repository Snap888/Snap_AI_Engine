# Telemetry logging

Suggested CSV:
```csv
frame_id,mode,profile,frame_ms,gpu_ms,motion,motion_complexity,flow_confidence,gpu_pressure,artifact_risk,flow_scale,search_radius_scale,refinement_passes,reuse_previous,bidirectional,artifact_check,ai_cpu_us,measured_flow_gpu_us,measured_total_gpu_us
```

Measure ai_cpu_us around snap_ai_update. GPU durations must come from GPU timestamp queries, never from the estimated saving field. Correlate the decision with the target frame explicitly if the host applies it one frame later. Keep AI logs separate from general renderer logs. Do not log images, personal identifiers or unrelated app state.

Suggested prefix:
`SnapAI: frame=<id> mode=<n> flow_scale=<x> confidence=<x> pressure=<x>`
