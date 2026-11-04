#ifndef SNAP_FRAME_MOTION_V2_H
#define SNAP_FRAME_MOTION_V2_H

#include <stdint.h>

#if defined(_WIN32)
  #if defined(SNAP_MOTION_V2_BUILD)
    #define SNAP_MOTION_V2_API __declspec(dllexport)
  #else
    #define SNAP_MOTION_V2_API __declspec(dllimport)
  #endif
#else
  #define SNAP_MOTION_V2_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SNAP_MOTION_V2_API_VERSION 1u

typedef struct SnapMotionV2 SnapMotionV2;

typedef enum SnapMotionV2Status {
  SNAP_MOTION_V2_OK = 0,
  SNAP_MOTION_V2_NO_DATA = 1,
  SNAP_MOTION_V2_ERROR_INVALID_ARGUMENT = -1,
  SNAP_MOTION_V2_ERROR_VERSION_MISMATCH = -2,
  SNAP_MOTION_V2_ERROR_STALE_METRICS = -3
} SnapMotionV2Status;

typedef enum SnapMotionV2Quality {
  SNAP_MOTION_V2_QUALITY_ECONOMY = 0,
  SNAP_MOTION_V2_QUALITY_BALANCED = 1,
  SNAP_MOTION_V2_QUALITY_QUALITY = 2,
  SNAP_MOTION_V2_QUALITY_ULTRA = 3
} SnapMotionV2Quality;

typedef enum SnapMotionV2Sharpen {
  SNAP_MOTION_V2_SHARPEN_OFF = 0,
  SNAP_MOTION_V2_SHARPEN_LOW = 1,
  SNAP_MOTION_V2_SHARPEN_MEDIUM = 2
} SnapMotionV2Sharpen;

typedef enum SnapMotionV2ResetReason {
  SNAP_MOTION_V2_RESET_MANUAL = 0,
  SNAP_MOTION_V2_RESET_RESOLUTION = 1,
  SNAP_MOTION_V2_RESET_FORMAT = 2,
  SNAP_MOTION_V2_RESET_SWAPCHAIN = 3,
  SNAP_MOTION_V2_RESET_FRAME_GAP = 4,
  SNAP_MOTION_V2_RESET_ENGINE_SWITCH = 5,
  SNAP_MOTION_V2_RESET_DEVICE_LOST = 6
} SnapMotionV2ResetReason;

typedef struct SnapMotionV2Config {
  uint32_t struct_size;
  uint32_t api_version;
  uint32_t enabled;
  SnapMotionV2Quality quality;
  uint32_t static_mask_enabled;
  uint32_t gamut_clamp_enabled;
  uint32_t validation_enabled;
  uint32_t temporal_seed_enabled;
  SnapMotionV2Sharpen sharpen;
  uint32_t adaptive_quality_enabled;
} SnapMotionV2Config;

/* Metrics are tagged with the source frame that produced them. They are
 * populated by an asynchronous backend reduction; this portable module never
 * reads Vulkan resources or performs GPU synchronization. */
typedef struct SnapMotionV2Metrics {
  uint32_t struct_size;
  uint64_t frame_id;
  uint64_t timestamp_ns;
  uint32_t width;
  uint32_t height;
  float motion_magnitude_px;
  float motion_complexity;
  float flow_confidence;
  float low_confidence_ratio;
  float occlusion_ratio;
  float static_ratio;
  float artifact_risk;
  uint32_t scene_cut;
  uint32_t telemetry_valid;
} SnapMotionV2Metrics;

SNAP_MOTION_V2_API void snap_motion_v2_default_config(SnapMotionV2Config* config);
SNAP_MOTION_V2_API SnapMotionV2* snap_motion_v2_create(const SnapMotionV2Config* config);
SNAP_MOTION_V2_API void snap_motion_v2_destroy(SnapMotionV2* motion);
SNAP_MOTION_V2_API SnapMotionV2Status snap_motion_v2_set_config(
    SnapMotionV2* motion, const SnapMotionV2Config* config);
SNAP_MOTION_V2_API SnapMotionV2Status snap_motion_v2_reset(
    SnapMotionV2* motion, SnapMotionV2ResetReason reason);
/* Rejects invalid, duplicate, or out-of-order frame metrics. */
SNAP_MOTION_V2_API SnapMotionV2Status snap_motion_v2_publish_metrics(
    SnapMotionV2* motion, const SnapMotionV2Metrics* metrics);
/* Returns only a valid sample not newer than current_frame_id and no older
 * than max_frame_lag frames. This lets the host consume async GPU readback
 * without ever pairing metrics with the wrong source frame. */
SNAP_MOTION_V2_API SnapMotionV2Status snap_motion_v2_get_metrics(
    SnapMotionV2* motion, uint64_t current_frame_id, uint32_t max_frame_lag,
    SnapMotionV2Metrics* metrics);
SNAP_MOTION_V2_API uint32_t snap_motion_v2_get_api_version(void);

#ifdef __cplusplus
}
#endif
#endif
