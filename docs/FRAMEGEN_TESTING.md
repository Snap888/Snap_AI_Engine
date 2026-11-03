# SnapFrame testing

## Automated portable tests

The mock backend test covers disabled/bypass, backend dispatch, hot enable and quality update, scene-cut reset, monotonic frame-id validation and statistics.

```sh
cmake -S . -B build -DSNAP_FRAME_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Shader validation

The four imported GLSL compute shaders must compile for Vulkan 1.1 and pass `spirv-val`. Use `framegen/tools/compile_snapframe_shaders.py` after configuring its source/output paths for the new repository layout.

## DXVK / Android device validation (not performed by this migration)

- Android ARM64 NDK build of the backend.
- Startup and clean shutdown.
- Resolution change / swapchain recreation.
- Device loss and reinitialization.
- Static scene, slow pan, fast rotation, moving NPCs, foliage/particles/water, HUD/inventory/map and long-run stress.
- Compare baseline vs generation-off vs generation-on.
- Inspect flow confidence, boundary saturation, consistency, HUD ghosting and disocclusion artifacts.
- Validate audio remains unchanged.
- Verify generated frame is presented only after successful GPU completion and original frame fallback remains intact.

A portable mock test does not prove shader validity, Vulkan correctness, Android ABI compatibility or visual quality. Record exact commit, APK, device, driver, settings and logs for every device claim. Original test report is retained in `docs/legacy/`.
