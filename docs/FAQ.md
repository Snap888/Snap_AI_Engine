# FAQ

### Is it a neural network?
No. v0.1 is a deterministic adaptive controller. Neural inference is a later optional milestone.

### Does it generate frames?
No. Snap Framegen performs optical flow, warping and reconstruction.

### Does it use Vulkan?
Not directly. It consumes scalar telemetry and returns policy recommendations. Host Vulkan code applies them.

### Does it reduce GPU usage?
It may recommend less flow work for simple, reliable scenes. Actual savings must be measured; no speedup is guaranteed.

### What if telemetry is missing?
Set telemetry_valid=0. The engine safely bypasses that frame.

### Does it work with every Winlator?
The library is host-agnostic, but forks differ. Integrate with the specific fork's existing adapter.

### Does it replace LSFG/WFG?
No. It is an optional controller for Snap Framegen; preserve existing backends.
