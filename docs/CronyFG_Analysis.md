# CronyFG source analysis

Source basis: user-supplied `cronyfg.zip` (12 archive entries).

## Source inventory

- `cronyfg_engine.cpp/.h`: orchestration wrapper; configures multiplier, target rate, flow resolution, refresh rate and quality preset; validates composite format as `VK_FORMAT_R8G8B8A8_UNORM`; prepares the chain and controls generation/pacing.
- `cronyfg_chain.cpp/.h`: owns Vulkan images, views, samplers, descriptor sets, pipelines, push constants and barriers.
- `cronyfg_quality.h`: six presets (Economy through Maximum) with flow short-side sizes 160, 208, 264, 320, 416 and 512 px and corresponding confidence/round-trip/gamut/sharpen tuning.
- Shaders: `cronyfg_luma.comp`, `cronyfg_downsample.comp`, `cronyfg_search.comp`, `cronyfg_refine.comp`, `cronyfg_interp.comp`.

## Algorithms observed

1. Luma conversion to R16F at flow resolution.
2. 2x2 box downsample to a half-resolution luma level.
3. Coarse block search at L1 followed by fine search at L0.
4. Temporal seed from previous refined flow, plus spatial-neighbour candidates.
5. Separate forward search for round-trip validation.
6. Refinement with 3x3 component median, subpixel refinement and confidence.
7. Bidirectional warp with confidence/edge fallback, neighborhood gamut clamp and optional detail sharpening.

## Coupled responsibilities that must not be copied

The engine/chain also owns pacing, multiplier planning, image allocation, descriptor/pipeline setup, Vulkan synchronization and composite-ring assumptions. These belong to SnapFrame Core or the host adapter, not to portable SnapMotionV2.

## Integration risks to resolve

- CronyFG hard-requires RGBA8 composite format in the inspected engine wrapper.
- Its renderer contract states that the renderer retains previous/current real frames and uses ring capacity >= multiplier + 2.
- Its chain allocates persistent images at prepare time and expects GENERAL layout throughout its pass chain.
- Temporal flow is reused across frames; SnapFrame must attach an explicit history epoch/frame ID so late readbacks or old vectors cannot seed a new history.
- The supplied archive alone does not establish Android/Adreno compatibility, performance or visual quality on Skyrim.
