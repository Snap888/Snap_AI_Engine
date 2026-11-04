# SnapMotionV2 compute shaders

These shaders are a separate V2 namespace. CI compiles and validates them, but
they are not loaded by the Skyrim runtime and do not replace the V1 pipeline.

## Stage 2 — Luma and coarse-to-fine search

1. `snap_motion_v2_luma.comp`: real source frame -> L0 Rec.709 luma (R16F).
2. `snap_motion_v2_downsample.comp`: L0 -> L1 2x2 box average (R16F).
3. `snap_motion_v2_search.comp`: shared coarse/fine block-search kernel selected
   by push constants. Motion vectors remain in L0 pixel units at both levels.

The search shader rejects out-of-frame previous-image samples rather than
relying on sampler edge-clamp. Spatial seed coordinates are clamped before
sampling. Coarse search can use a valid previous refined field; fine search uses
the current frame's coarse field.

## Stage 3 — Refinement and validation

4. `snap_motion_v2_refine.comp`: componentwise 3x3 median, parabolic subpixel
   refinement, photometric cost confidence and local agreement confidence.
5. `snap_motion_v2_validation.comp`: forward/backward round-trip residual and
   a soft validity mask. Out-of-frame reprojections are invalid, not edge-clamped.

Refinement and validation are separate so their GPU cost can be measured
independently and validation can be toggled in controlled comparisons.

## Stage 4/5 — Artifact suppression and interpolation

6. `snap_motion_v2_static_mask.comp`: temporal stability mask from two real
   frames. It preserves static pixels but does not semantically label HUD.
7. `snap_motion_v2_interpolation.comp`: time-parametric bidirectional warp,
   confidence/validation fallback, 5-tap neighborhood gamut clamp, optional
   real-frame detail recovery and static-mask override. Output is RGBA16F.

## Stage 6 — GPU metrics reduction

8. `snap_motion_v2_metrics_partial.comp`: per-workgroup reduction of motion
   magnitude, local vector variation, confidence, low-confidence ratio,
   occlusion ratio, static ratio and mean photometric delta.
9. `snap_motion_v2_metrics_final.comp`: reduces the compact partial buffer into
   one frame-tagged scalar record. It emits a scene-cut *candidate* and marks
   telemetry invalid if fewer than 90% of expected flow cells were valid.

The reduction is two-pass to avoid global atomics. The host can asynchronously
copy only the final small record; it must not read back full flow vectors or wait
for the GPU. Artifact risk and scene-cut are heuristics and require device/scene
validation before they drive AI decisions.

## Descriptor and push-constant contract

All shaders use set 0, binding 0 for a primary sampled image. Most use binding 4
for storage output; interpolation uses bindings 0–4 for inputs and binding 5 for
its RGBA16F storage output.

- Luma push: `vec4(sourceW, sourceH, lumaW, lumaH)`.
- Downsample push: `vec4(lumaW, lumaH, halfW, halfH)`.
- Search push:
  - `dims = vec4(levelW, levelH, blocksX, blocksY)`
  - `geom = vec4(pixelScale, firstStep, minStep, useNeighbours)`
  - `flags.x = temporalHistoryValid`
- Refine push:
  - `dims = vec4(lumaW, lumaH, blocksX, blocksY)`
  - `tune = vec4(costScale, agreementScale, 0, 0)`
- Validation push:
  - `dims = vec4(lumaW, lumaH, blocksX, blocksY)`
  - `tune = vec4(roundTripLow, roundTripHigh, 0, 0)`

- Static-mask push: `dims = vec4(width, height, thresholdLow, thresholdHigh)`.
- Interpolation push:
  - `dims = vec4(dstW, dstH, lumaW, lumaH)`
  - `params.x = interpolationTime`
  - `tune = vec4(gamutStrength, confidenceFloor, sharpen, 0)`

Metrics partial push constants: none; dispatch dimensions define the flow grid.
The partial buffer stores two vec4 sums per workgroup. Metrics-final push constants:
`identity = uvec4(frameIdLo, frameIdHi, timestampLo, timestampHi)`,
`extent = uvec4(sourceW, sourceH, flowW, flowH)`, and
`counts = uvec4(partialCount, expectedFlowCells, 0, 0)`. The final SSBO record is
80 bytes: identity, extent, flags, motion metrics and quality metrics.

The host adapter owns descriptors, samplers, image layouts, barriers, async
readback and persistent resource lifetime. No pass may wait for GPU completion.
The shader family is a source-level port, not a drop-in pipeline: the host must
validate formats, filtering, workgroup limits, barriers and frame-history
ownership before dispatch.


## Coordinate normalization correction — 2026-11-04

All V2 motion vectors are expressed in L0 luma pixels. Validation must divide
reprojected L0 pixel positions by the L0 luma image extent before sampling the
normalized flow texture. Interpolation must likewise divide L0-pixel motion by
the L0 luma extent when converting displacement to normalized output UV.
Neither pass may use the smaller flow-grid width/height as the normalization
denominator. The validation and interpolation push-constant contracts above
therefore carry L0 luma dimensions, not flow-grid dimensions.
