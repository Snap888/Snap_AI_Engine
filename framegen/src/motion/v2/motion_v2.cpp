#include "snap_frame/motion_v2.h"

#include <cmath>
#include <cstring>
#include <mutex>
#include <new>

struct SnapMotionV2 {
  SnapMotionV2Config config{};
  SnapMotionV2Metrics latest{};
  std::mutex mutex;
  uint64_t last_published_frame = 0;
  bool has_metrics = false;
};

namespace {
bool validConfig(const SnapMotionV2Config* c) {
  return c && c->struct_size >= sizeof(SnapMotionV2Config)
      && c->api_version == SNAP_MOTION_V2_API_VERSION
      && c->enabled <= 1u
      && c->quality >= SNAP_MOTION_V2_QUALITY_ECONOMY
      && c->quality <= SNAP_MOTION_V2_QUALITY_ULTRA
      && c->static_mask_enabled <= 1u
      && c->gamut_clamp_enabled <= 1u
      && c->validation_enabled <= 1u
      && c->temporal_seed_enabled <= 1u
      && c->sharpen >= SNAP_MOTION_V2_SHARPEN_OFF
      && c->sharpen <= SNAP_MOTION_V2_SHARPEN_MEDIUM
      && c->adaptive_quality_enabled <= 1u;
}

bool normalized(float v) {
  return std::isfinite(v) && v >= 0.0f && v <= 1.0f;
}

bool validMetrics(const SnapMotionV2Metrics* m) {
  return m && m->struct_size >= sizeof(SnapMotionV2Metrics)
      && m->frame_id != 0 && m->timestamp_ns != 0
      && m->width != 0 && m->height != 0
      && std::isfinite(m->motion_magnitude_px) && m->motion_magnitude_px >= 0.0f
      && normalized(m->motion_complexity)
      && normalized(m->flow_confidence)
      && normalized(m->low_confidence_ratio)
      && normalized(m->occlusion_ratio)
      && normalized(m->static_ratio)
      && normalized(m->artifact_risk)
      && m->scene_cut <= 1u
      && m->telemetry_valid == 1u;
}
}

extern "C" void snap_motion_v2_default_config(SnapMotionV2Config* c) {
  if (!c) return;
  std::memset(c, 0, sizeof(*c));
  c->struct_size = sizeof(*c);
  c->api_version = SNAP_MOTION_V2_API_VERSION;
  c->enabled = 0;
  c->quality = SNAP_MOTION_V2_QUALITY_BALANCED;
  c->static_mask_enabled = 1;
  c->gamut_clamp_enabled = 1;
  c->validation_enabled = 1;
  c->temporal_seed_enabled = 1;
  c->sharpen = SNAP_MOTION_V2_SHARPEN_OFF;
  c->adaptive_quality_enabled = 0;
}

extern "C" SnapMotionV2* snap_motion_v2_create(const SnapMotionV2Config* config) {
  SnapMotionV2Config c{};
  if (config) c = *config; else snap_motion_v2_default_config(&c);
  if (!validConfig(&c)) return nullptr;
  auto* motion = new (std::nothrow) SnapMotionV2();
  if (!motion) return nullptr;
  motion->config = c;
  return motion;
}

extern "C" void snap_motion_v2_destroy(SnapMotionV2* motion) {
  delete motion;
}

extern "C" SnapMotionV2Status snap_motion_v2_set_config(
    SnapMotionV2* motion, const SnapMotionV2Config* config) {
  if (!motion || !validConfig(config))
    return SNAP_MOTION_V2_ERROR_INVALID_ARGUMENT;
  std::lock_guard<std::mutex> lock(motion->mutex);
  motion->config = *config;
  return SNAP_MOTION_V2_OK;
}

extern "C" SnapMotionV2Status snap_motion_v2_reset(
    SnapMotionV2* motion, SnapMotionV2ResetReason reason) {
  if (!motion || reason < SNAP_MOTION_V2_RESET_MANUAL
      || reason > SNAP_MOTION_V2_RESET_DEVICE_LOST)
    return SNAP_MOTION_V2_ERROR_INVALID_ARGUMENT;
  std::lock_guard<std::mutex> lock(motion->mutex);
  motion->latest = SnapMotionV2Metrics{};
  motion->has_metrics = false;
  /* Keep last_published_frame monotonic across resets: late readbacks from an
   * old history epoch must not be accepted after the reset. */
  return SNAP_MOTION_V2_OK;
}

extern "C" SnapMotionV2Status snap_motion_v2_publish_metrics(
    SnapMotionV2* motion, const SnapMotionV2Metrics* metrics) {
  if (!motion || !validMetrics(metrics))
    return SNAP_MOTION_V2_ERROR_INVALID_ARGUMENT;
  std::lock_guard<std::mutex> lock(motion->mutex);
  if (metrics->frame_id <= motion->last_published_frame)
    return SNAP_MOTION_V2_ERROR_STALE_METRICS;
  motion->latest = *metrics;
  motion->last_published_frame = metrics->frame_id;
  motion->has_metrics = true;
  return SNAP_MOTION_V2_OK;
}

extern "C" SnapMotionV2Status snap_motion_v2_get_metrics(
    SnapMotionV2* motion, uint64_t current_frame_id, uint32_t max_frame_lag,
    SnapMotionV2Metrics* metrics) {
  if (!motion || !metrics || metrics->struct_size < sizeof(*metrics)
      || current_frame_id == 0)
    return SNAP_MOTION_V2_ERROR_INVALID_ARGUMENT;
  std::lock_guard<std::mutex> lock(motion->mutex);
  if (!motion->has_metrics)
    return SNAP_MOTION_V2_NO_DATA;
  if (motion->latest.frame_id > current_frame_id
      || current_frame_id - motion->latest.frame_id > max_frame_lag)
    return SNAP_MOTION_V2_ERROR_STALE_METRICS;
  const uint32_t requestedSize = metrics->struct_size;
  *metrics = motion->latest;
  metrics->struct_size = requestedSize;
  return SNAP_MOTION_V2_OK;
}

extern "C" uint32_t snap_motion_v2_get_api_version(void) {
  return SNAP_MOTION_V2_API_VERSION;
}
