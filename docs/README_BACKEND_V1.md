# SnapFrameBackendV1 developer guide

## Design boundary

SnapFrame core is portable orchestration. The backend is the graphics API integration. The backend owns every API-specific resource, queue operation, barrier and generated-frame insertion. The host owns its render lifecycle and final presentation policy.

## Data flow

1. Host identifies a real frame and supplies stable metadata plus an opaque native token.
2. SnapFrame core validates frame order/configuration and invokes backend processing.
3. Backend records or schedules GPU passes and returns stage timings and generated-frame count.
4. Host accepts the generated image at its existing pacing/present boundary.
5. Snap AI may consume asynchronously sampled scalar telemetry and suggest bounded settings.

## Error and fallback

If initialization fails, disable frame generation and leave the original renderer untouched. If a frame fails, present the real frame. On resize/scene cut/device loss, reset temporal history. Never expose a partially written midpoint image.

## Hot settings

The UI may change settings while the game runs. Queue changes to the render thread. The adapter applies cheap toggles immediately at a safe boundary; settings that alter image sizes, descriptors or pipelines are rebuilt only after in-flight work is safe. Persist desired settings separately from applied settings and log both.

## Diagnostics

Keep separate engine and backend logs. Recommended event fields: monotonic timestamp, frame id, backend, device/driver, dimensions, format, config generation, bypass reason, stage GPU timings, generated count and device-loss/reset events. Avoid per-frame CPU readback and high-volume flow-vector dumps unless an explicit diagnostic mode is active.
