# SnapFrame and Snap AI validation

## Portable tests

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
~~~

Test core lifecycle, config, frame ordering, bypass, output-count bounds, hot settings, scene reset and invalid inputs. Shader CI must compile GLSL for the declared Vulkan target and validate generated SPIR-V.

## Algorithm invariants

- Flow is solved once per consecutive source pair regardless of ×2/×3/×4 multiplier.
- Phase-specific synthesis dispatch count matches generated phases actually supported.
- Generated phases are distinct and chronological.
- No stale flow after scene cut, frame gap, resize, format change, device loss or swapchain generation change.
- Debug-off mode adds no diagnostic dispatch/readback.
- Invalid metrics bypass safely; no fabricated confidence/risk values.

## X11 and Wayland host tests

Run separately. Verify host device/queue/command-buffer ownership; real-frame fallback; first-slot and additional-slot WSI timeout policy; acquire timeout/error handling without treating VK_TIMEOUT as device loss; generated output ordering and successful presents; resize/swapchain recreation; format/HDR negotiation; enable/disable; scene transitions; device loss/recovery; long sessions; audio and unrelated renderer paths unchanged.

## GSFG A/B suite

Use identical physical hardware, driver, game build, scene route, resolution, refresh and graphics settings. Start within comparable thermal range, alternate order and run at least three captures. Record exact commit/APK and logs.

Measure quality against a real high-frame-rate reference midpoint using PSNR/SSIM plus edge/occlusion/temporal metrics. Measure real source FPS, generated compute count, successful generated presents, acquire misses, pacing, latency, per-stage GPU timings, CPU overhead and sustained thermal/power behavior.

A generated-frame counter is not a display counter. Lower GPU time is not a win if artifacts or dropped frames increase. Portable CI and mock backends do not prove physical-device image quality or scanout.
