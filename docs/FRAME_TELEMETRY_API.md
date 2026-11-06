# Frame presentation telemetry API

Header: `include/snap_ai/frame_telemetry.h`  
Implementation: `src/frame_telemetry.cpp`  
Namespace: `snap_ai::telemetry`

This is a portable, passive recorder inspired by frame-pacing diagnostics. It is
a clean-room implementation: it does not include or link Special K and has no
DXGI, Win32, Vulkan, DXVK, Android, X11 or Wayland dependency.

## What it records

Each accepted `PresentEvent` contains:

| Field | Contract |
|---|---|
| `logical_present_id` | Nonzero ID unique across both source and generated streams for one logical displayed event. Reuse the same ID for nested/wrapped callbacks of that same present. |
| `frame_id` | Nonzero host frame identifier. |
| `monotonic_time_ns` | Nonzero monotonic timestamp in nanoseconds, sampled at one consistent presentation boundary. |
| `cpu_duration_ns` | Optional CPU-side duration; zero means unavailable. |
| `swapchain_generation` | Generation owned by the host; increment on swapchain/device recreation. |
| `stream` | `Source` or `Generated`; classify what was actually presented. |
| `api` | Diagnostic origin: `Present`, `Present1` or `Vulkan`. |

The host must record only successful/accepted presentations at the boundary that
represents display delivery. Do not count command-buffer submission, frame
generation completion, acquire success or a failed/out-of-date present as a
displayed frame.

## Minimal integration

```cpp
#include "snap_ai/frame_telemetry.h"

snap_ai::telemetry::Recorder recorder(256);
uint64_t swapchain_generation = 1;

// Call after the host confirms a source image was successfully presented.
snap_ai::telemetry::PresentEvent e{};
e.logical_present_id = next_logical_present_id++;
e.frame_id = source_frame_id;
e.monotonic_time_ns = monotonic_now_ns;
e.cpu_duration_ns = present_cpu_duration_ns; // or 0 if unavailable
e.swapchain_generation = swapchain_generation;
e.stream = snap_ai::telemetry::Stream::Source;
e.api = snap_ai::telemetry::PresentApi::Vulkan;
recorder.record(e);

// Poll from a diagnostics/UI thread, not from a latency-sensitive callback.
const auto stats = recorder.snapshot();
```

The clock and successful-present signal are host responsibilities. The recorder
does not call a clock, intercept a graphics API, submit GPU work, block on GPU
readback, pace frames or create another frame-generation loop.

## Duplicate accounting

A DXGI wrapper may observe both `Present` and nested `Present1` for one
logical presentation. Either instrument only the outermost callback, or submit
the same nonzero `logical_present_id` from both callbacks. The second event is
rejected and increments `duplicate_events`. IDs must be unique across streams
within the recent-ID window; do not reuse a source ID for a generated frame.

The recorder retains the last 256 logical IDs for duplicate detection. An ID
reused after it ages out can be accepted, so hosts should use monotonically
increasing IDs or another non-repeating sequence for the lifetime of the
recorder.

## Swapchain lifecycle

Start with generation 1 (or another host-owned nonzero generation). Increment
the generation whenever the output swapchain or underlying device is recreated.
Call `reset(new_generation)` after quiescing event producers. A first event
with a higher generation also advances the recorder and clears the rolling
sample window. Events from lower generations are rejected and increment
`stale_generation_events`.

Reset clears the rolling samples and duplicate-ID history but preserves
lifetime diagnostic counters. Create a new recorder if a completely fresh
counter epoch is required.

## Snapshot semantics

- `source.presents` and `generated.presents`: accepted events in the current
  rolling sample window.
- `interval_median_ms`, `interval_p95_ms`, `interval_p99_ms`: intervals
  between successive accepted events in that stream, in insertion order.
- `present_cpu_median_ms`, `present_cpu_p95_ms`: nonzero CPU duration samples.
- `output_interval_*`: intervals between all accepted source and generated
  events, in insertion order.
- `accepted_events`, `duplicate_events`, `stale_generation_events`,
  `invalid_events`: lifetime counters for this recorder instance.
- Percentiles return 0 when there are no valid intervals/samples. The API does
  not return FPS; a host may derive an estimate from a sufficiently populated
  interval series, while clearly labeling it as an estimate.

The sample window is bounded by the configured capacity (minimum 8; default
256). Percentiles are computed when `snapshot()` is called. `record()` uses a
mutex and performs no dynamic allocation or graphics/GPU operation;
`snapshot()` allocates temporary CPU vectors and sorts samples, so call it
from a diagnostics/UI thread rather than the presentation hot path. This is
thread-safe but not lock-free.

Timestamps must be monotonic and events should be submitted in presentation
order. Out-of-order timestamps are ignored for interval deltas (they do not
produce a negative interval); the implementation does not reorder samples.

## What this API does not prove

The portable recorder and unit tests validate event accounting and statistics.
They do not prove that a host adapter is observing the correct display boundary,
that a generated image reached the display, or that physical display scanout
matches the event timestamp. Host integration and physical-device validation
are separate milestones. See [WinNative integration](INTEGRATION_WINNATIVE.md)
and [testing](FRAMEGEN_TESTING.md).
