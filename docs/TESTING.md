# Testing and validation

## Automated tests
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Unit tests cover default config, handle creation, warmup bypass, bounded outputs, scene-cut reset, invalid telemetry, enable/disable, profile validation, stats and invalid config rejection.

## Integration matrix
| Case | Profile | Scene | Expected |
|---|---|---|---|
| AI Off | — | fixed route | Baseline unchanged |
| Performance | Performance | static/simple | lower flow tier only at high confidence |
| Balanced | Balanced | mixed | stable policy, no rapid oscillation |
| Quality | Quality | fast motion | more validation/refinement when uncertain |
| Scene cut | any | hard cut | bypass/reset |
| Invalid metrics | any | missing telemetry | bypass |
| GPU saturated | any | high pressure | pressure governor limits work |

## Device protocol
Use identical device, APK, scene, resolution, graphics settings and thermal starting condition. Run at least three captures per mode. Record source/generated FPS, flow GPU time, total generation GPU time, frame pacing, power/thermal state, artifact rate and AI CPU time. Compare median and p95. The estimated saving field is only a policy heuristic; calculate real savings from GPU timestamps.

## Production acceptance
- no crash or Vulkan validation errors;
- AI-Off output/performance unchanged;
- no stale history across cuts/resolution changes;
- stable policy under pans and scene transitions;
- repeatable measured GPU improvement greater than controller overhead;
- no unacceptable visual artifact regression;
- sustained thermal behavior no worse.

No Android device test is claimed by the standalone repository.
