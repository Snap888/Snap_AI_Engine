# Migration record

Source: `Snap888/Skyrim_Android`, branch `frame_gen`, commit `c385a66daef920f83a86e7eaf16ff822e507e3c7`.

Imported:
- `android/app/jni/snapframe/snapframe_host.cpp`, host/types headers.
- Four GLSL compute shaders.
- Example config and shader compiler script.
- SnapFrame readme, design, Vulkan hybrid architecture and test report.

The imported host is preserved under `framegen/backends/dxvk/legacy`; historic docs under `docs/legacy`. It is not represented as portable: it directly references DXVK internals and links to DXVK D3D9.

The new portable C ABI/core is separate and is the stable boundary for future adapters. Migration is staged to avoid a risky rewrite of the GPU pipeline in one change.
