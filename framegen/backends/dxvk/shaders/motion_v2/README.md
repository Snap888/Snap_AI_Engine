# SnapMotionV2 compute shaders

These shaders are a separate V2 namespace. They are compiled and validated by CI,
but are not loaded by the Skyrim runtime and do not replace the V1 pipeline.

## Stage 2 passes

1. `snap_motion_v2_luma.comp`: real source frame -> L0 Rec.709 luma (R16F).
2. `snap_motion_v2_downsample.comp`: L0 -> L1 2x2 box average (R16F).
3. `snap_motion_v2_search.comp`: one coarse/fine block-search kernel selected by
   push constants. Motion vectors remain in L0 pixel units at both levels.

The search shader explicitly rejects out-of-bounds previous-frame samples rather
than relying on sampler edge-clamp. Spatial seed coordinates are clamped before
sampling. Coarse search can use a valid previous refined field; fine search uses
the current frame's coarse field.

## Descriptor and push-constant contract

All three shaders use set 0, binding 0 for the primary sampled image and binding
4 for the storage output. Search additionally uses binding 1 for previous luma
and binding 2 for the seed field. The host adapter owns descriptors, samplers,
image layouts, barriers and persistent resource lifetime.

- Luma push: `vec4(sourceW, sourceH, lumaW, lumaH)`.
- Downsample push: `vec4(lumaW, lumaH, halfW, halfH)`.
- Search push:
  - `dims = vec4(levelW, levelH, blocksX, blocksY)`
  - `geom = vec4(pixelScale, firstStep, minStep, useNeighbours)`
  - `flags.x = temporalHistoryValid`

The current CronyFG search/refine shader family is a reference for algorithm
transfer, not a drop-in Vulkan pipeline. The host must validate image format,
sampler filtering, workgroup limits and frame-history ownership before dispatch.
