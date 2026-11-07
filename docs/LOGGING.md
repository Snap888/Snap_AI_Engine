# SnapFrame and Snap AI telemetry logging

Logs must distinguish source frames, generated-frame computation, WSI acquisition and successful presentation. These are not interchangeable counters.

## Recommended per-frame CSV

~~~csv
frame_id,phase_id,multiplier,profile,mode,source_interval_ms,flow_scale,flow_gpu_ms,consistency_gpu_ms,warp_gpu_ms,total_gpu_ms,motion_mean,motion_deviation,fb_weighted_confidence,consistency_risk_proxy,telemetry_valid,ai_cpu_us,acquire_attempts,acquire_successes,acquire_timeouts,generated_present_success,generated_present_fail,swapchain_generation,refresh_hz
~~~

- GPU times come from Vulkan timestamp queries; identify the query slot/frame ID.
- Flow metrics are asynchronous reductions; log the source-pair frame ID they describe.
- consistency_risk_proxy is diagnostic, not calibrated artifact probability.
- Acquisition and present counters must be recorded at actual host boundaries.
- A generated image produced by compute is not a displayed frame.
- Do not report scanout time unless measured by an appropriate display mechanism.

## X11 / Wayland diagnostics

Use the same semantic field names in both paths, with separate platform tags. Current acquisition policy: X11 uses a refresh-aware first generated-slot wait bounded to 8–16 ms, with 1 ms best-effort waits for additional slots. Wayland uses one refresh interval clamped to 4–16 ms for the first slot (16 ms if refresh is unknown) and 1 ms for subsequent slots. These are host-side bounds, not guaranteed presentation intervals.

## Suggested log events

~~~text
SnapFrame: source=<id> multiplier=<M> flow_ms=<x> warp_ms=<x> total_ms=<x>
SnapAI: source=<id> flow_scale=<x> motion=<x> complexity=<x> confidence=<x> risk_proxy=<x> pressure=<x> valid=<0|1>
SnapWSI: platform=<x11|wayland> source=<id> slot=<n> acquire=<success|timeout|error> present=<success|failed>
~~~

Report median/P90/P95, not only averages or maxima. Include real source FPS, generated compute count, successful generated presents, acquire miss ratio, CPU present duration, GPU stage timings, device/driver, output format, refresh rate, thermal state and exact commit/APK. Never infer actual speedup from estimated_compute_saving.
