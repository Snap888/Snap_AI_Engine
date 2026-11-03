# Snap AI Engine

Lightweight adaptive policy engine for frame-generation pipelines. The first release, AI-Lite v0.1, is a deterministic telemetry-driven controller; it is **not** a trained neural network and does not generate/interpolate pixels.

## Purpose
- Reduce unnecessary optical-flow work in simple, high-confidence scenes.
- Spend more compute on uncertain or complex motion.
- Apply GPU-pressure limits and temporal hysteresis.
- Keep the controller optional, explainable and fail-safe.
- Share one C ABI across Skyrim_Android, DXVK-based Winlator forks and WinNative's Android Vulkan compositor.

## Architecture
The host renderer owns capture, Vulkan objects, synchronization, optical flow and reconstruction. Snap AI Engine receives scalar telemetry and returns bounded recommendations for the *next* flow dispatch. It never owns images, queues, command buffers, semaphores or swapchains.

## Build
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSNAP_AI_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Android NDK:
```bash
cmake -S . -B build-android \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26 \
  -DCMAKE_BUILD_TYPE=Release -DSNAP_AI_BUILD_TESTS=OFF
cmake --build build-android --parallel
```

Builds `libsnap_ai_engine.so` by default; set `-DBUILD_SHARED_LIBS=OFF` for a static archive.

## Quick API example
```cpp
#include <snap_ai/snap_ai.h>
SnapAIConfig cfg{};
snap_ai_default_config(&cfg);
cfg.profile = SNAP_AI_PROFILE_BALANCED;
SnapAIHandle* ai = snap_ai_create(&cfg);

SnapAIFrameMetrics m{};
m.struct_size = sizeof(m);
m.frame_id = 1;
m.frame_time_ms = 41.6f;
m.gpu_time_ms = 3.8f;
m.motion_magnitude = 0.2f;
m.motion_complexity = 0.3f;
m.flow_confidence = 0.9f;
m.gpu_pressure = 0.55f;
m.artifact_risk = 0.05f;
m.telemetry_valid = 1;

SnapAIDecision d{};
d.struct_size = sizeof(d);
if (snap_ai_update(ai, &m, &d) == SNAP_AI_OK) {
    // Map decision to validated host flow tiers; apply next frame only.
}
snap_ai_destroy(ai);
```

## Profiles
- **Performance** — prioritize reduced workload.
- **Balanced** — default compromise.
- **Quality** — preserve/add flow refinement when uncertainty is high.

## Project status
AI-Lite C++17 controller, C ABI, build system, sample host and unit tests are the v0.1 target. Integration adapters for Skyrim_Android, DXVK, Winlator and WinNative must be added and validated in their respective repositories. No Android device benchmark is claimed by this repository.

## Documentation
- [Architecture](docs/ARCHITECTURE.md)
- [API](docs/API.md)
- [Integration: Skyrim / DXVK / Winlator / WinNative](docs/INTEGRATION.md)
- [Algorithms and future neural backend](docs/ALGORITHMS.md)
- [Testing and benchmark protocol](docs/TESTING.md)
- [Telemetry logging](docs/LOGGING.md)
- [Roadmap](docs/ROADMAP.md)
- [FAQ](docs/FAQ.md)

## Safety guarantees
Invalid telemetry, scene cuts, disabled state and non-monotonic frame IDs return a bypass decision. The host must clamp every recommendation to device-supported settings. AI output must never control Vulkan synchronization or resource ownership. If the library is unavailable or fails, use the existing baseline renderer unchanged.

## License
MIT. See [LICENSE](LICENSE).
