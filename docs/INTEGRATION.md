# Integration: Skyrim_Android, DXVK, Winlator, WinNative

## General adapter contract
- Create a handle per rendering context.
- Read measured timestamps and trustworthy flow telemetry.
- Call update after metrics are available.
- Map decision values to supported discrete tiers and clamp to device limits.
- Apply decisions to the next frame only.
- Log decision and measured outcome.
- On failure or disabled mode, retain baseline behavior.

Do not scale Vulkan image dimensions blindly. Calibrate tier mapping to the flow buffers, shader assumptions, alignment and workgroup sizes. AI must never choose barriers, semaphore waits, queue ownership or present timing.

## Skyrim_Android
Recommended first integration target. Add optional settings: Off / Performance / Balanced / Quality. Build ARM64 shared library with Android NDK or link static archive. Initialize after Vulkan context and Snap Framegen settings. Feed real GPU timestamps and flow confidence. Apply policy to next flow dispatch. Preserve LSFG and all existing audio behavior; Snap AI must not touch audio. Add separate AI telemetry logging and compare AI OFF against each profile.

This repository does not yet include a patch to Skyrim_Android. Exact current frame_gen call sites must be audited before changing that repository.

## DXVK
Keep integration in a thin optional Snap Framegen adapter, not generic DXVK present logic. DXVK can provide source-frame IDs and GPU timestamps; adapter passes metrics to Snap AI and forwards recommendations to Snap Framegen Core. Preserve generic DXVK behavior when the library is absent. Build option should default OFF until validated.

## Winlator
Winlator forks differ. Integrate in the specific fork's existing Snap Framegen/DXVK adapter. Package ARM64 .so or statically link it; verify NDK, ABI, STL and loader paths. Expose settings using the fork's UI. Validate DXVK and WineD3D paths separately.

## WinNative
Preferred integration point is Android-side Vulkan compositor's frame-generation controller, because it owns frame timing and Vulkan resources. Snap AI remains a policy library; compositor owns images, descriptors, command buffers and synchronization. Preserve LSFG/WFG modes and add Snap AI only as optional controller for Snap Framegen. Check third-party licenses before distributing combined binaries.

## Other hosts
Any renderer with trustworthy scalar telemetry and a tunable flow pipeline can use the C ABI. Host remains responsible for API capture, UI, packaging, GPU synchronization and device capability checks.
