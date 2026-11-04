#ifdef NDEBUG
#undef NDEBUG
#endif
#include "snap_frame/motion_v2.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>

int main() {
  SnapMotionV2Config config{};
  snap_motion_v2_default_config(&config);
  assert(config.api_version == SNAP_MOTION_V2_API_VERSION);
  assert(config.enabled == 0 && config.quality == SNAP_MOTION_V2_QUALITY_BALANCED);
  assert(config.static_mask_enabled && config.gamut_clamp_enabled);
  SnapMotionV2* motion = snap_motion_v2_create(&config);
  assert(motion);

  SnapMotionV2Metrics out{};
  out.struct_size = sizeof(out);
  assert(snap_motion_v2_get_metrics(motion, 10, 2, &out) == SNAP_MOTION_V2_NO_DATA);

  SnapMotionV2Metrics sample{};
  sample.struct_size = sizeof(sample);
  sample.frame_id = 8;
  sample.timestamp_ns = 1000000;
  sample.width = 2448;
  sample.height = 1080;
  sample.motion_magnitude_px = 3.5f;
  sample.motion_complexity = 0.42f;
  sample.flow_confidence = 0.75f;
  sample.low_confidence_ratio = 0.12f;
  sample.occlusion_ratio = 0.04f;
  sample.static_ratio = 0.61f;
  sample.artifact_risk = 0.18f;
  sample.telemetry_valid = 1;
  assert(snap_motion_v2_publish_metrics(motion, &sample) == SNAP_MOTION_V2_OK);

  out.struct_size = sizeof(out);
  assert(snap_motion_v2_get_metrics(motion, 10, 2, &out) == SNAP_MOTION_V2_OK);
  assert(out.frame_id == 8 && out.motion_magnitude_px == 3.5f);
  out.struct_size = sizeof(out);
  assert(snap_motion_v2_get_metrics(motion, 11, 2, &out) == SNAP_MOTION_V2_ERROR_STALE_METRICS);
  assert(snap_motion_v2_publish_metrics(motion, &sample) == SNAP_MOTION_V2_ERROR_STALE_METRICS);

  sample.frame_id = 9;
  sample.motion_complexity = std::numeric_limits<float>::quiet_NaN();
  assert(snap_motion_v2_publish_metrics(motion, &sample) == SNAP_MOTION_V2_ERROR_INVALID_ARGUMENT);
  sample.motion_complexity = 0.2f;
  sample.frame_id = 9;
  assert(snap_motion_v2_publish_metrics(motion, &sample) == SNAP_MOTION_V2_OK);

  assert(snap_motion_v2_reset(motion, SNAP_MOTION_V2_RESET_SWAPCHAIN) == SNAP_MOTION_V2_OK);
  out.struct_size = sizeof(out);
  assert(snap_motion_v2_get_metrics(motion, 9, 1, &out) == SNAP_MOTION_V2_NO_DATA);
  sample.frame_id = 8; // Late result from the previous history epoch.
  assert(snap_motion_v2_publish_metrics(motion, &sample) == SNAP_MOTION_V2_ERROR_STALE_METRICS);

  config.adaptive_quality_enabled = 2;
  assert(snap_motion_v2_set_config(motion, &config) == SNAP_MOTION_V2_ERROR_INVALID_ARGUMENT);
  snap_motion_v2_destroy(motion);
  return 0;
}
