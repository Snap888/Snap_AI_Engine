# Roadmap: surpass GSFG through measured engineering

The product objective is to surpass GSFG in image quality, successful frame delivery, latency, GPU cost, pacing, power/thermal efficiency, robustness and supported host paths. This is a target with explicit acceptance gates, not a present performance claim. See [GSFG competitive plan](GSFG_COMPETITIVE_PLAN.md).

## Current baseline — implemented or integrated

- Portable SnapFrame core and versioned backend ABI.
- Host-owned Vulkan interop contract.
- WinNative X11 direct VulkanRenderer integration and separate Wayland SnapFrameBackendV1 adapter.
- Luma/pyramid, bidirectional motion, fusion, consistency and phase-specific reconstruction pipeline.
- Up to ×2/×3/×4 API/scheduling capability, with up to three generated phases.
- Flow reuse across phases of one source pair.
- GPU timestamps and asynchronous flow reductions in WinNative backend.
- Snap AI deterministic policy with bounded settings and hysteresis.
- X11 Debug Flow modes: motion, matcher confidence and forward/backward consistency.
- Bounded generated-slot WSI acquire waits on X11 and Wayland.

Check the active host branch and latest CI before claiming a build passed. Portable tests do not prove device behavior.

## P0 — close the delivery feedback loop

- Instrument acquire attempts, success, timeout, other errors, successful generated presents and swapchain generation in both X11 and Wayland.
- Validate counters against actual host counters and captured output.
- Feed rolling WSI delivery ratio into a shared Snap AI feature set.
- Reduce generation multiplier/slot budget when delivery is poor; restore gradually after stable periods.
- Add deterministic tests for timeout bursts, swapchain recreation, stale IDs, partial delivery and recovery.

## P1 — complete flow intelligence and quality signals

- Improve complexity from magnitude deviation to edge-aware spatial vector disagreement.
- Add separate occlusion/disocclusion ratio, photometric residual and search-boundary saturation.
- Calibrate confidence/risk against held-out true midpoint frames.
- Add selectable views for confidence, occlusion, FB error, candidate warps and blend weights; keep debug-off path free of extra dispatches.
- Add equivalent safe diagnostics in Wayland only through its own host UI.

## P2 — algorithmic quality and compute efficiency

- A/B global-motion pre-warp and edge-preserving flow regularization independently.
- Improve candidate selection and occlusion-aware fallback.
- Preserve one flow solve per source pair for ×2/×3/×4; measure phase synthesis cost separately.
- Tune flow scale, search and reconstruction by device capability and sustained thermal budget.
- Consider asynchronous compute only where profiling proves overlap and synchronization cost does not erase the gain.

## P3 — benchmark and optional learning

- Build opt-in lossless source/ground-truth triplet capture with manifest and hashes.
- Split datasets by game/scene/session; never use generated outputs as ground truth.
- Establish repeatable GSFG A/B harness: same device, driver, resolution, refresh, scene, settings and thermal starting state.
- Evaluate a compact residual corrector only if it improves quality at net GPU/latency benefit; keep it optional and classical fallback intact.

## P4 — platform completion and release

- Validate X11 and Wayland separately on physical Turnip/Adreno hardware.
- Complete generic Winlator runtime adapter for a precisely identified fork and DXVK/compositor version.
- Validate Skyrim_Android/DXVK adapter independently.
- Verify resize, format/HDR, scene discontinuity, device loss, long sessions, UI/HUD, audio unaffected and clean fallback.
- Publish exact APK, commit, device, driver, logs and reproducible benchmark methodology.

## Release gates

No claim of beating GSFG until repeated A/B runs show statistically meaningful improvement without unacceptable regressions in image quality, pacing, latency, power or stability. Report wins and losses per scenario.
