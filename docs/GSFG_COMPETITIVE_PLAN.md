# GSFG competitive engineering plan

## Objective

The target is to surpass GSFG as a complete frame-generation system, not merely to produce a larger generated-FPS counter. SnapFrame must demonstrate measurable advantages in image quality, successful displayed-frame delivery, latency, GPU cost, pacing, thermal efficiency, stability and platform coverage.

This document is a benchmark and development plan. It does not claim that the current implementation already beats GSFG.

## Fair comparison rules

For every A/B pair, hold constant:
- physical device, Android build and GPU driver;
- game build, scene route, camera path and source frame cadence;
- resolution, render scale, output format, refresh rate and graphics settings;
- thermal starting range and background workload;
- capture duration and warm-up period.

Alternate test order to reduce thermal/order bias. Run at least three captures per mode and report median plus P90/P95. Preserve exact APK, commit SHA, configuration and raw logs.

## Scorecard

| Area | Required measurements | Win condition |
|---|---|---|
| Image quality | True-midpoint PSNR/SSIM, edge error, occlusion-boundary error, temporal stability, HUD ghosting | Better quality at equivalent output cadence, with no severe regression in difficult scenes |
| Frame delivery | Source frames, generated compute frames, acquire success, successful generated presents, dropped/missed frames | More successfully displayed generated frames, not merely more compute completions |
| Latency | Input-to-display where measurable, present queue delay, CPU present duration | Lower or equal latency at comparable quality |
| GPU efficiency | Flow GPU time, synthesis GPU time, total GPU time per additional displayed frame | Lower cost for equivalent or better quality |
| Pacing | Inter-present interval median/P95/P99, variance, bursts and stalls | More stable cadence with fewer long gaps |
| Power/thermal | Sustained clocks, thermal state, power where measurable, performance after 15–30 min | Less throttling and no sustained regression |
| Robustness | Crashes, device loss, swapchain recreation, scene cuts, format changes, fallback | No black/stale frames; predictable recovery |
| Compatibility | X11, Wayland, HDR/SDR, Adreno/Turnip and explicitly supported host paths | Broader validated support without fragile special cases |

## SnapFrame-specific advantages to prove

1. **Flow reuse:** one forward/backward flow solution per source pair, reused for all temporal phases. Verify dispatch counts for ×2/×3/×4.
2. **Adaptive workload:** Snap AI uses measured GPU time and validated flow/quality signals to avoid spending compute where it does not improve output.
3. **Delivery-aware generation:** combine WSI acquire/present success with GPU budget so the system does not overproduce frames the host cannot display.
4. **Candidate reconstruction:** compare candidate warp/reconstruction strategies using real ground truth, not visual guesswork.
5. **Host-native scheduling:** preserve host queue, swapchain and presentation ownership; avoid a second competing presenter.
6. **Diagnostics:** expose motion, confidence, consistency, occlusion and candidate views without adding cost when disabled.

## Immediate implementation priorities

### P0 — WSI delivery feedback
- Record per-slot acquire attempts, successes, timeouts and errors in X11 and Wayland.
- Record only successful generated presents as delivered output.
- Correlate events to source frame, phase and swapchain generation.
- Feed a rolling delivery ratio into a common Snap AI policy.
- Reduce multiplier/slot count when delivery misses rise; restore slowly after stable windows.
- Keep Vulkan synchronization and present decisions in the host.

### P1 — calibrated quality signals
- Add edge-aware vector complexity, explicit occlusion/disocclusion ratio, photometric residual and search-boundary saturation.
- Calibrate confidence/risk against true high-rate midpoint frames.
- Keep invalid features explicitly invalid.

### P2 — visual quality experiments
- A/B global-motion pre-warp and edge-preserving regularization independently.
- Evaluate candidate reconstruction and occlusion-aware fallback.
- Require per-stage GPU timing and ground-truth quality improvement before enabling by default.

### P3 — sustainable performance
- Tune flow scale and phase synthesis by device capability and sustained thermal budget.
- Test asynchronous compute only where profiling proves a net gain.
- Avoid neural inference in the critical path unless it demonstrably saves more time than it costs.

## Ground truth and artifact evaluation

Use high-frame-rate source captures with the actual middle frame as target. A generated SnapFrame or GSFG frame is not ground truth. Split validation by game/scene/session, not adjacent frames. Include static scenes, slow pans, fast rotation, thin geometry, foliage, particles, water, HUD, disocclusion and scene cuts.

## Reporting template

Every published comparison should include:
- device/GPU/driver/OS;
- exact game build, APK and commit;
- resolution, refresh, format and engine settings;
- scene and capture duration;
- source FPS, generated compute count and successful generated presents;
- acquire miss ratio and dropped frames;
- GPU stage median/P90/P95 and CPU present timing;
- quality metrics and representative artifact crops;
- thermal/power behavior;
- regressions and failed scenarios.

Do not claim “beats GSFG” from one scene, one run, requested multiplier or generated-frame counter. A win must be repeatable and must not trade away image quality, latency, power or stability.
