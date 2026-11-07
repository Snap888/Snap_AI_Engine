# Snap AI policy tests

Build:

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSNAP_AI_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
~~~

Policy tests cover config validation, lifecycle, warmup, bounded outputs, enable/disable, profile validation, invalid telemetry, scene-cut reset, frame ordering, stats and safe bypass.

## Required policy cases

| Case | Expected behavior |
|---|---|
| AI disabled | Baseline SnapFrame settings unchanged |
| High GPU pressure | Clamp workload within configured bounds |
| High motion / low confidence | Avoid unsafe aggressive flow reduction |
| Invalid/missing metric | Safe bypass; never fabricate a value |
| Scene cut/frame gap | Reset temporal policy/history |
| Rapidly changing metrics | Hysteresis prevents tier oscillation |
| Backend/library failure | Real-frame path remains available |
| WSI miss burst (planned closed loop) | Reduce generated slot/multiplier budget, then recover slowly |
| X11 vs Wayland | Same policy semantics; host synchronization remains separate |

## Host and device validation

Portable tests do not prove shader correctness or GPU presentation. Host validation must include Android ARM64 APK build; X11 and Wayland separately; device/GPU/driver; resolution/format/refresh; thermal start state; static scenes, pans, fast rotation, moving objects, foliage/particles, water, HUD, inventory/map and scene cuts; source FPS; generated compute count; successful generated presents; acquire miss ratio; GPU stage percentiles; resize/swapchain recreation; enable/disable; device loss/recovery; long-run stability; and audio/unrelated paths unchanged.

## GSFG A/B protocol

Use identical physical hardware, driver, game build, scene route, resolution, refresh and graphics settings. Start within comparable thermal range, alternate run order and run at least three captures. Record exact commit/APK and logs.

Measure quality against a real high-frame-rate reference midpoint using PSNR/SSIM plus edge/occlusion/temporal metrics. Measure real source FPS, generated compute count, successful generated presents, acquire misses, pacing, latency, per-stage GPU timings, CPU overhead and sustained thermal/power behavior.

A generated-frame counter is not a display counter. Lower GPU time is not a win if artifacts or dropped frames increase. Portable mock tests are not a GSFG benchmark.
