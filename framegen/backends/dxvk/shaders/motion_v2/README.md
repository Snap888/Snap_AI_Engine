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

## Descriptor and push-constant contract

All shaders use set 0, binding 0 for a primary sampled image and binding 4 for
the storage output.

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
  - `dims = vec4(flowW, flowH, blocksX, blocksY)`
  - `tune = vec4(roundTripLow, roundTripHigh, 0, 0)`

The host adapter owns descriptors, samplers, image layouts, barriers, async
readback and persistent resource lifetime. No pass may wait for GPU completion.
The shader family is a source-level port, not a drop-in pipeline: the host must
validate formats, filtering, workgroup limits, barriers and frame-history
ownership before dispatch.
