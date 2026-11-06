# Roadmap

## v0.1 — AI-Lite reference (implemented)

C ABI, C++17 policy controller, profiles, smoothing, hysteresis, pressure governor, safe bypass, build, tests, example and integration documentation.

## v0.1.x — Portable presentation telemetry (implemented)

- Public C++ `Recorder` API and bounded sample ring.
- Source/generated stream classification and duplicate logical-present rejection.
- Swapchain-generation tracking and stale-event rejection.
- Median/p95/p99 interval and CPU present-duration snapshots.
- Deterministic unit tests and host integration documentation.

This milestone is a passive recorder only. It does not hook a graphics API, calculate FPS, alter pacing or imply host presenter integration.

## v0.2 — Host presentation adapters (next)

- Wire the recorder to successful outermost presentation events in WinNative X11 and Wayland.
- Add source/generated logical IDs and swapchain generation lifecycle.
- Compare event counts with existing host counters and captured output.
- Add diagnostic overlay fields after the event boundary is validated.
- Keep present mode, queue depth and existing single SnapFrame pipeline unchanged.

## v0.3 — Better motion telemetry

Forward/backward consistency, occlusion and scene-cut signals, per-region complexity aggregation, confidence calibration from real logs. Replace neutral host priors only when trustworthy measurements are available.

## v0.4 — Optional tiny model

Dataset and feature schema, offline training/evaluation, quantized model, optional runtime adapter, inference budget and fallback tests.

## v1.0 — Cross-host release

Stable ABI, Android ARM64/Windows/Linux builds, validated Skyrim_Android, DXVK, Winlator fork and WinNative compositor adapters. Physical-device validation and license review remain release gates.

Roadmap items are plans unless explicitly marked implemented. Portable CI, host compilation and physical runtime validation are separate milestones.
