# Performance, telemetry and optimization

## Optimize the whole displayed-frame path

Measure capture/history, luma/pyramid, forward/backward flow and fusion, consistency/metric reductions, phase-specific warp/candidate reconstruction, composite/copy, swapchain acquisition/present, CPU render/present overhead and thermal behavior.

For ×2/×3/×4, report flow solve count separately from synthesis phase count. The intended invariant is one flow solve per source pair, then phase-specific synthesis for each generated output.

## GPU timing

Use Vulkan timestamp queries and correlate each result to source-pair/frame ID. Collect non-blockingly after completion. Report median, P90 and P95, not only average/max. Timestamp-derived GPU budget pressure is not hardware utilization, queue backlog or thermal state.

Historical Skyrim measurements and older logs are useful for identifying bottlenecks but are not current WinNative benchmarks. Never transfer timings across devices, drivers, resolutions or branches without qualification.

## Flow reductions and Snap AI

WinNative asynchronously reduces four values: mean normalized flow magnitude; magnitude deviation (complexity proxy); forward/backward-consistency-weighted confidence; and forward-confidence inconsistency risk proxy. The bounded staging/query path avoids synchronous full-frame readback.

Magnitude deviation is not full edge-aware complexity. Risk is not calibrated artifact probability. Further features should include occlusion ratio, photometric residual, vector-direction variation and search-boundary saturation.

## Presentation and WSI

Do not equate generated compute completion, successful swapchain acquisition, successful queue submission, successful present and physical display scanout.

X11/Wayland bounded acquire policy gives the first generated slot a refresh-aware wait and limits additional slots to 1 ms. This protects the host from serial long waits but does not guarantee delivery. Measure acquire success/miss ratio and successful generated presents separately.

## GSFG comparison

Use the same device, driver, scene, resolution, refresh, game settings and thermal starting condition. Alternate test order and repeat captures. Compare image quality against true midpoint ground truth, successfully displayed output rate, dropped/missed generated frames, pacing variance, latency, CPU overhead, GPU time per additional displayed frame, power/thermal behavior and long-run stability.

The target is to beat GSFG across a balanced suite, not by optimizing one headline FPS number. Publish scenarios where SnapFrame loses as well as wins.
