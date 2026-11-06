# Special K-inspired frame telemetry

This work takes architectural inspiration from Special K's presentation diagnostics and frame-pacing measurements. It is a **clean-room, portable implementation**: no Special K source code is copied, linked or required. The upstream Special K project is GPL-3.0; this repository does not embed it.

## Implemented in Snap AI Engine

- `include/snap_ai/frame_telemetry.h`: public C++ API.
- `src/frame_telemetry.cpp`: bounded recorder, duplicate-event rejection, swapchain-generation checks and rolling statistics.
- `tests/test_frame_telemetry.cpp`: deterministic unit tests for source/generated streams, duplicate Present/Present1 accounting, interval percentiles, stale generations, reset and invalid input.
- `docs/FRAME_TELEMETRY_API.md`: host adapter contract and usage guide.

The recorder exposes source and generated event counts, per-stream interval median/p95/p99, CPU present-duration median/p95 and combined output interval percentiles. It does not calculate FPS; consumers may derive an estimate from sufficient interval samples and must label it accordingly.

## Host integration contract

The host adapter owns all graphics API calls and submits an event only at its successful presentation boundary. Each event supplies:
- a stable logical ID, shared by nested/wrapped callbacks for one logical present;
- a nonzero monotonic timestamp in nanoseconds;
- source/generated classification;
- a host-owned swapchain generation;
- optional CPU-side present duration.

The recorder deduplicates logical IDs (last 256), rejects events from stale swapchain generations and keeps a bounded rolling sample window. `record()` uses a mutex and preallocated storage; it is not lock-free. `snapshot()` allocates temporary CPU vectors and sorts samples, so it belongs on a diagnostics/UI thread.

The recorder never intercepts Present/Present1/Vulkan, calls a graphics API, delays a frame, changes queue depth, performs GPU readback or runs a second pacing/FG loop. Hosts must instrument only the outermost callback unless nested callbacks share one logical ID.

## Current integration status (2026-11-07)

- Snap AI Engine: implemented and covered by portable tests.
- WinNative: recorder source/header are vendored into the `snap_ai` static target on `Snap888-patch-1`; actual X11/Wayland presenter call sites are **not yet wired**. It must not be shown as live game FPS telemetry yet.
- Skyrim_Android and Winlator: no host presentation recorder adapter is claimed.
- Physical display/scanout timing: not measured by this API.

## Next adapter work

1. Instrument successful source and generated presentations at each host's outermost presentation boundary.
2. Assign one monotonically increasing logical ID per displayed event; share IDs only across wrappers for the same event.
3. Increment swapchain generation on recreation/device replacement and reject failed/out-of-date presents.
4. Poll snapshots outside the present hot path and expose metrics in the diagnostic overlay.
5. Validate event counts against host counters and device-side frame capture before using the data for AI training or pacing changes.

Do not change present mode, queue depth, frame scheduling or SnapFrame generation as part of telemetry instrumentation. Keep existing host synchronization and the single FG pipeline unchanged.
