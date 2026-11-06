# Performance and telemetry

The previous Skyrim_Android device report records flow estimation as the dominant compute cost. In one 2448x1080 capture, forward and backward flow were each around 5 ms median; the total chain was around 13 ms median. These are historical measurements from the preserved report, not portable benchmarks or guaranteed current timings.

Measure per-stage GPU timestamps on the backend: capture copy, luma, forward flow, backward flow, consistency, warp, composite and total chain. Compare identical scenes, same resolution, same build and same device state. Record median/P90, dropped frames, real FPS and output FPS.

## Presentation timing

The portable `snap_ai::telemetry::Recorder` stores host-reported successful source/generated present events and computes rolling interval median/p95/p99 plus CPU present-duration median/p95. It is passive and does not infer presentation from GPU submission or frame-generation completion.

A host must instrument the actual successful presentation boundary, supply monotonic timestamps and stable logical IDs, and track swapchain generations. Do not call its interval values physical scanout measurements. The recorder itself does not calculate FPS; a host may derive an estimated rate from sufficiently populated intervals.

`record()` is mutex-protected and uses bounded preallocated storage; it is not lock-free. Poll `snapshot()` from a diagnostics/UI thread because snapshot calculation allocates temporary vectors and sorts the bounded window. See [Frame telemetry API](FRAME_TELEMETRY_API.md).

Do not interpret heuristic estimates as actual savings. Lower flow scale can reduce compute but may lower motion quality; quantify both.
