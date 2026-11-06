# Special K-inspired telemetry (clean-room implementation)

This module implements portable frame-presentation telemetry concepts. It does
not include Special K source code and has no dependency on Windows, DXGI, Vulkan,
DXVK, Android, or a particular window system.

## Host integration contract

The host adapter remains the owner of Present/Present1/Vulkan present calls.
Immediately after the host has assigned a logical presentation ID, it may submit
a `PresentEvent` with:
- a stable `logical_present_id` shared by wrapper/nested Present and Present1
  callbacks for one logical presentation;
- monotonically increasing host timestamp in nanoseconds;
- source/generated stream classification;
- swapchain generation, incremented whenever the swapchain is recreated;
- optional CPU-side present duration.

The recorder deduplicates by logical presentation ID, rejects stale generations,
and maintains bounded rolling samples. It does not intercept, call, delay, or
synchronize a graphics API. The host must not call it twice with unrelated IDs
for one logical present; adapters that cannot identify nested calls should
instrument only the outermost call.

## Metrics

Snapshots expose source and generated present counts, interval median/p95/p99,
CPU present-duration median/p95, and combined output interval percentiles.
Percentiles are computed from the bounded sample window, not lifetime totals.
No FPS is fabricated when there are insufficient intervals (zero is returned).

## Swapchain/device lifecycle

Call `reset(new_generation)` after swapchain recreation or device replacement.
Events from older generations are rejected. Keep recorder lifetime scoped to the
host/presenter instance and stop submissions before destroying it.

## Integration status

The recorder and deterministic unit tests are implemented in the portable
Snap AI Engine. This is a telemetry primitive, not yet a live hook in WinNative's
X11/Wayland presenters. Wiring those two distinct adapters requires their
respective outermost present sites and must preserve their existing queue,
semaphore, and resource ownership. Do not install an extra Present hook or
second frame-generation/pacing loop.
