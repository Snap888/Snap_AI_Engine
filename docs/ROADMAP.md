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

## v0.3 — Flow reuse and quality diagnostics (priority)

- Preserve and verify the existing WinNative flow/synthesis split: forward/backward flow is solved once per source pair and reused by each generated phase. Add dispatch counters/regression tests and ensure the same invariant holds in every host adapter.
- Invalidate cached flow on scene cut, frame gap, resize, format/device/swapchain generation change.
- The X11 host already has an opt-in full-frame HSV flow + consistency diagnostic view. Improve it with selectable confidence, occlusion, warp-candidate and blend-weight views; expose an equivalent Wayland control only through its own safe host UI.
- Add GPU-side reductions for motion magnitude/complexity, confidence, occlusion ratio, photometric residual and search-boundary saturation.
- Add global-motion pre-warp and edge-preserving flow regularization as independently switchable experiments, with per-stage GPU timestamps and A/B quality reports.

See [Win-FG technical audit](WIN_FG_TECHNICAL_AUDIT.md) and [flow diagnostics/data contract](FLOW_DIAGNOSTICS_AND_DATASET.md). These are planned until implemented and device-validated in the active host backend.

## v0.4 — Calibration and optional tiny model

- Opt-in lossless capture of real source frames and true high-rate midpoint targets, with manifest, hashes and explicit consent.
- Offline confidence calibration against held-out ground-truth scenes.
- Dataset splits by game/scene/session to avoid adjacent-frame leakage.
- Evaluate a small optional residual corrector only after classical baseline measurements; keep classical flow/warp/synthesis as default and fallback.
- Quantized model, measured inference budget and heuristic fallback tests if a model demonstrates net benefit on target Adreno hardware.

Do not use generated SnapFrame outputs as ground truth. Do not block the render thread on readback, compression or file I/O. Do not silently upload captured gameplay.

## v1.0 — Cross-host release

Stable ABI, Android ARM64/Windows/Linux builds, validated Skyrim_Android, DXVK, Winlator fork and WinNative compositor adapters. Physical-device validation and license review remain release gates.

Roadmap items are plans unless explicitly marked implemented. Portable CI, host compilation and physical runtime validation are separate milestones.
