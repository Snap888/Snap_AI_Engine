# FAQ

### Is Snap AI a neural network?
No. The current controller is deterministic AI-Lite policy logic with smoothing, GPU-budget limits and hysteresis. A learned model is optional future research, not a runtime requirement.

### Does Snap AI generate frames?
No. SnapFrame performs motion estimation, warping, candidate reconstruction and temporal phase synthesis. Snap AI only recommends bounded settings.

### Does SnapFrame support ×2/×3/×4?
The portable contract allows multiplier 1 (bypass) and 2..4 (up to three generated phases). Actual host/backend capability and output scheduling must be checked; the API range is not proof every platform supports every mode.

### Does it solve flow again for every generated frame?
The design reuses the source-pair flow solution and performs phase-specific synthesis for each generated phase. Keep this invariant covered by dispatch-count tests.

### Does AI understand image quality?
Partially. WinNative feeds asynchronous GPU reductions for motion magnitude, magnitude deviation, consistency-weighted confidence and a consistency-risk proxy, plus GPU timestamps. The risk is not calibrated artifact probability and complexity is not yet a full edge-aware measure.

### Does AI control WSI or presentation?
No. Host code owns acquisition, semaphores, queue submission and presentation. WSI delivery feedback into AI is planned, not yet a closed loop.

### Is X11 the same as Wayland?
No. WinNative has separate X11 VulkanRenderer and Wayland compositor paths. Shared compute does not make their present loops, UI controls or validation interchangeable.

### Is it already better than GSFG?
That is the project objective, not a verified claim. Only reproducible A/B tests on the same device and scenes can establish that.

### Does it work in every Winlator?
No universal runtime adapter is complete. A specific Winlator fork and renderer/DXVK version must be integrated and validated.

### What if telemetry or Vulkan support is missing?
Mark telemetry invalid or disable the unsupported backend and retain the host real-frame/baseline path.
