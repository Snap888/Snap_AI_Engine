# Performance and telemetry

The previous Skyrim_Android device report records flow estimation as the dominant compute cost. In one 2448x1080 capture, forward and backward flow were each around 5 ms median; the total chain was around 13 ms median. These are historical measurements from the preserved report, not portable benchmarks or guaranteed current timings.

Measure per-stage GPU timestamps on the backend: capture copy, luma, forward flow, backward flow, consistency, warp, composite and total chain. Compare identical scenes, same resolution, same build and same device state. Record median/P90, dropped frames, real FPS and output FPS.

Do not interpret heuristic estimates as actual savings. Lower flow scale can reduce compute but may lower motion quality; quantify both.
