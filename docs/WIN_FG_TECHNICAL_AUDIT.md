# Win-FG technical audit and SnapFrame integration plan

Audit date: 2026-11-07  
Reviewed repository: [Snap888/win-fg](https://github.com/Snap888/win-fg), branch `master`, commit `fbc69ed6c5bf16b824d0a29bd8b171675101f9dd`.

## Executive decision

Use Win-FG as an algorithmic and diagnostics reference. Do **not** run Win-FG and SnapFrame as two independent frame generators in the same presentation path. Keep one owner for generated-frame scheduling, output slots, queue submission, WSI synchronization and presentation: the selected host adapter running SnapFrame.

The highest-value transfer is not a second optical-flow implementation. It is a better division of work inside the existing SnapFrame pipeline:

1. Estimate forward/backward motion once for each pair of real source frames.
2. Retain the flow, consistency/occlusion and any refinement data for that source pair.
3. Generate the requested temporal phases from those shared fields, with phase-specific warp/composite work only.
4. Invalidate that history on scene cuts, resize, format changes, swapchain/device generation changes or non-consecutive source frames.

This is the first optimization to prototype because Win-FG's own README identifies repeated flow recomputation for each generated phase as its largest remaining multiplier-cost gap. SnapFrame already exposes multiplier 2..4 and chronological output slots, so the optimization belongs inside the backend contract/implementation, not in a second host or presenter.

## What was inspected

- Win-FG README, CMake/build layout, shader sources, framegen interfaces, capture implementation, provenance and research notes.
- Win-FG's public fork metadata: MIT license; fork of `The412Banner/win-fg`. The reviewed snapshot includes FSR3 optical-flow adaptations with a separate AMD FidelityFX MIT attribution notice.
- SnapFrameBackendV1 public API, Vulkan host interop contract, portable core, AI policy inputs, frame telemetry API and current DXVK legacy backend.
- Current Snap_AI_Engine branch head at audit start: `7954a835043467a9305a114faa434ad5d6ee263a`.

## Transfer candidates

| Priority | Win-FG idea | SnapFrame action | Why / acceptance gate |
|---|---|---|---|
| P0 | Flow/synthesis split | Cache one flow solution per consecutive source pair; synthesize all temporal phases from it | Major 3x/4x compute reduction. Verify identical flow dispatch count for 2x/3x/4x on the same source pair. |
| P1 | Global-motion pre-warp (C1) | Estimate camera/global affine motion, stabilize luma before local flow, add global motion back during reconstruction | Targets camera-pan melt and large coherent motion. Must not introduce visible latency or GPU readback in render path. |
| P1 | Forward/backward consistency and disocclusion | Feed a confidence/occlusion mask into synthesis and fallback selection | Reduces ghosting and invalid warps near silhouettes. Measure against true midpoint frames. |
| P1 | Robust flow filtering / TV-L1 regularization (C2) | Median/outlier rejection and edge-preserving regularization as independently gateable passes | Improve speckle and incoherent vectors while preserving object edges. Benchmark GPU cost and quality separately. |
| P2 | Flow pyramid/search preset | Expose validated discrete quality/performance tiers to Snap AI | Adapt search cost only when measured pressure and flow confidence justify it. |
| P2 | Debug views | Add opt-in vector/confidence/occlusion/residual views in host diagnostics | Visual debugging and calibration; must be absent from normal presentation path. |
| P3 | Capture for learning | Add opt-in lossless source-triplet capture and manifest tooling | Build an owned, consented dataset; no automatic upload and no render-thread disk I/O. |

## Architecture mapping

Win-FG has both a Vulkan layer path and a native compositor path. Its README explains why generated layer presents can be discarded by a downstream compositor that retains only the newest buffer. SnapFrame must not reproduce that split.

SnapFrame's current ownership contract is the correct boundary:

- Host owns Vulkan device, queue, command buffer, swapchain, output slots, WSI semaphores, submission and presentation.
- SnapFrame backend records compute work into the host-owned command buffer and writes only to host-provided output slots.
- SnapFrame core validates frame order/configuration and exposes up to three generated phases.
- Snap AI consumes measured scalar features and returns bounded policy suggestions. It must never own Vulkan synchronization or decide whether a frame is safe to present.

The existing `SnapFrameVulkanFrame` is a borrowed per-callback payload. A backend may retain its own GPU resources only under an explicit device/swapchain-generation lifetime contract; it must not retain borrowed command buffers or host handles after the callback.

## Important current limitation

The portable repository contains the API/core and a preserved DXVK legacy implementation. That legacy implementation is explicitly not the new standalone `SnapFrameBackendV1` adapter. The active WinNative Vulkan compute implementation is host-owned in the WinNative repository, not in this repository. Therefore this audit does **not** claim that C1/C2, a vector viewer, flow reuse, or a trained model have been implemented in the active Android/WinNative runtime.

Implementing those shaders only as detached files here would create an unintegrated second pipeline. The next code change must be made in the active Vulkan backend and its host adapter, then reflected back into the portable contract only where a host-neutral API is genuinely needed.

## AI: use measured features, not pixels in the policy loop

The existing Snap AI v1 metrics already have slots for motion magnitude, motion complexity, flow confidence, GPU pressure and artifact risk. Current WinNative integration still supplies neutral priors for several motion/quality values. The right next step is to populate those values from GPU-side reductions and timestamp results, then calibrate them against offline reference frames.

Recommended feature set (all normalized to [0,1], with explicit validity):

- motion magnitude and p50/p95 flow magnitude;
- flow confidence and forward/backward residual;
- estimated occlusion/disocclusion ratio;
- photometric warp residual;
- fraction of vectors clamped at search-window boundaries;
- spatial flow variation / edge disagreement;
- scene-cut score;
- measured per-stage GPU time and GPU-pressure estimate.

Keep raw flow images and debug overlays out of the AI hot path. Reduce them on GPU to a small feature buffer, then consume results asynchronously after completion. If a metric is unavailable, mark telemetry invalid rather than fabricate a neutral value and treat it as measured truth.

## Licensing and provenance

The reviewed Win-FG repository is MIT-licensed, and the reviewed optical-flow shader set explicitly adapts AMD FidelityFX FSR3 optical-flow passes under MIT. This is compatible in principle with Snap_AI_Engine's MIT license, provided the exact source files copied are checked individually and required copyright/license notices are retained.

Do not copy opaque SPIR-V blobs or code whose source/provenance is not established. Prefer implementation from published algorithms or audited source files. Preserve the FidelityFX attribution when adapting any FSR3-derived code. Record every imported file, upstream commit, license and local modifications in `docs/NOTICE.md` / third-party notices before code is copied.

## Implementation sequence

1. **Backend audit and flow reuse prototype:** identify the active backend's real flow dispatch boundary; cache flow per source pair and separate phase synthesis. Add dispatch counters and compare 2x/3x/4x.
2. **Debug visualization:** implement debug-only output modes for flow direction/magnitude, confidence, FB error, occlusion, warped candidates and final residual. Prefer a host overlay or selectable diagnostic output image; do not add another presenter.
3. **Quality signals:** add GPU reductions for the feature set above. Feed only validated scalar metrics to Snap AI.
4. **C1/C2 experiments:** port global-motion pre-warp and robust regularization as opt-in passes; A/B each independently on the same captures.
5. **Capture and calibration:** opt-in lossless capture of real source triplets, manifest, consent and offline ground-truth scoring.
6. **Optional learned component:** evaluate a tiny residual corrector only after the classical flow baseline is measured. Keep classical flow/warp/synthesis as the fallback and default.

## Required validation

- No second frame-generation pipeline or second present hook.
- No synchronous GPU readback, allocation, compression or file I/O on the render/present hot path.
- One flow solve per real source pair regardless of multiplier; phase-specific synthesis count equals generated output count.
- Correct invalidation on scene cut, frame gap, resize, format/device/swapchain generation changes.
- Source frame always remains a safe fallback.
- A/B logs include source FPS, generated FPS, per-stage GPU median/P90, flow dispatches, skipped frames, confidence, occlusion and artifact metrics.
- Visual evaluation includes static scene, slow pan, fast rotation, thin geometry, foliage/particles, HUD and scene cuts.
- Validate on the target Adreno/Turnip device; portable CI alone is not proof of runtime image quality.
