# Win-FG technical audit and SnapFrame transfer plan

## Purpose and boundary

This document records algorithmic ideas worth evaluating from the sibling Win-FG project. SnapFrame remains the single frame-generation pipeline. Do not add a second generator, presenter or flow estimator simply to reproduce an upstream architecture.

## Transfer candidates

| Priority | Idea | SnapFrame direction | Acceptance gate |
|---|---|---|---|
| P0 | Flow/synthesis split | Solve motion once per consecutive source pair; reuse for each phase | Same flow dispatch count for ×2/×3/×4 |
| P1 | Global-motion pre-warp | Stabilize coherent camera motion before local flow and restore it during synthesis | Less pan distortion without unacceptable GPU cost |
| P1 | Forward/backward consistency and disocclusion | Confidence/occlusion-aware source selection | Lower ghosting at silhouettes against true midpoint |
| P1 | Robust flow filtering / TV-L1-style regularization | Independently gate outlier rejection and edge-preserving regularization | Better edge quality at measured cost |
| P2 | Flow pyramid/search presets | Map validated tiers to Snap AI | Stable quality under measured GPU budget |
| P2 | Debug views | Motion/confidence/occlusion/candidate views | No extra normal-path work when disabled |
| P3 | Capture for learning | Opt-in lossless source/ground-truth triplets | Valid consent, hashes and held-out evaluation |

## Architecture decision

Host owns Vulkan device, queue, command buffer, swapchain, output slots, WSI semaphores, submission and presentation. SnapFrame records compute into host-owned command buffers. Snap AI consumes scalar telemetry and returns bounded recommendations. No second present hook or competing frame scheduler.

## Current WinNative findings

- Flow reuse across phases is already part of the active backend design; preserve it and add dispatch-count regression tests rather than introducing another cache/estimator.
- X11 has selectable flow diagnostics for motion, matcher confidence and forward/backward consistency.
- WinNative exposes asynchronous GPU reductions for mean motion magnitude, magnitude deviation, consistency-weighted confidence and a confidence/consistency risk proxy.
- X11 and Wayland have separate host paths and now use bounded WSI acquisition waits: X11 first generated slot 8–16 ms and subsequent slots 1 ms; Wayland first slot 4–16 ms (16 ms when refresh is unknown) and subsequent slots 1 ms.
- These acquire bounds do not yet close the loop into Snap AI. WSI delivery telemetry is a priority.

## Remaining quality signals

Add edge-aware vector complexity, explicit occlusion/disocclusion ratio, photometric residual and search-boundary saturation. Validate every signal against true high-rate midpoint ground truth. Do not label a confidence/consistency proxy as artifact probability.

## Licensing and provenance

Review the exact upstream file, revision and license before copying any implementation. Preserve required attribution, including AMD FidelityFX notices for any adapted FSR3-derived code. Do not import opaque SPIR-V or assets with unclear provenance. Record every imported file and modification in NOTICE before redistribution.

## Validation

- one flow solve per source pair regardless of multiplier;
- phase-specific synthesis count matches generated outputs;
- no stale history across scene cut, frame gap, resize, format/device/swapchain changes;
- no synchronous GPU readback or render-thread disk I/O;
- successful generated presents, not compute completions, determine delivery;
- compare against GSFG using the same device, scene, settings and thermal conditions;
- physical Turnip/Adreno tests are required; portable CI is not device validation.
