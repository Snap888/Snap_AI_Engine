# Snap AI Engine

The current SnapFrame + Snap AI implementation and its complete documentation are maintained on the [SnapFrameBackendV1 branch](https://github.com/Snap888/Snap_AI_Engine/tree/SnapFrameBackendV1).

## Current project direction

- SnapFrame is the frame-generation engine; Snap AI is the optional adaptive policy controller.
- WinNative has separate X11 VulkanRenderer and Wayland compositor integrations.
- The engine targets ×2/×3/×4 temporal phase generation, flow reuse, GPU telemetry and adaptive workload control.
- The strategic goal is to surpass GSFG on image quality, successful frame delivery, latency, GPU cost, pacing, power/thermal efficiency and robustness. This is a target, not a verified performance claim.

## Documentation and implementation status

Use the [current branch README](https://github.com/Snap888/Snap_AI_Engine/blob/SnapFrameBackendV1/README.md) as the authoritative overview. It links to architecture, API, algorithms, X11/Wayland integration, testing, telemetry, competitive plan and roadmap.

The default branch retains the original AI-Lite v0.1 reference implementation. It should not be treated as the latest SnapFrame integration snapshot.

## Build (AI-Lite reference on main)

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSNAP_AI_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
~~~

For current SnapFrame build instructions and integration constraints, follow the SnapFrameBackendV1 branch.
