#include "snap_ai/snap_ai.h"
#include <cassert>
#include <cstdio>

static SnapAIFrameMetrics make_metrics(uint64_t id, float complexity, float confidence, float pressure) {
  SnapAIFrameMetrics m{}; m.struct_size = sizeof(m); m.frame_id = id;
  m.frame_time_ms = 33.3f; m.gpu_time_ms = pressure * 5.0f;
  m.motion_magnitude = complexity; m.motion_complexity = complexity;
  m.flow_confidence = confidence; m.gpu_pressure = pressure;
  m.artifact_risk = 0.0f; m.telemetry_valid = 1; return m;
}
static SnapAIDecision update(SnapAIHandle* h, SnapAIFrameMetrics m) {
  SnapAIDecision d{}; d.struct_size = sizeof(d);
  assert(snap_ai_update(h, &m, &d) == SNAP_AI_OK); return d;
}
int main() {
  SnapAIConfig c{}; snap_ai_default_config(&c);
  assert(c.api_version == SNAP_AI_API_VERSION);
  SnapAIHandle* h = snap_ai_create(&c); assert(h);
  auto d = update(h, make_metrics(1, .1f, .98f, .1f));
  assert(d.mode == SNAP_AI_MODE_BYPASS);
  for (uint64_t i=2; i<=20; ++i) d = update(h, make_metrics(i,.05f,.99f,.1f));
  assert(d.flow_scale >= c.min_flow_scale && d.flow_scale <= c.max_flow_scale);
  assert(d.refinement_passes <= c.max_refinement_passes);
  auto cut = make_metrics(21,.9f,.2f,.8f); cut.scene_cut = 1;
  d = update(h, cut); assert(d.mode == SNAP_AI_MODE_BYPASS);
  auto invalid = make_metrics(22,.1f,.9f,.1f); invalid.telemetry_valid = 0;
  d = update(h, invalid); assert(d.mode == SNAP_AI_MODE_BYPASS);
  assert(snap_ai_set_enabled(h,0) == SNAP_AI_OK);
  d = update(h, make_metrics(23,.1f,.99f,.1f)); assert(d.mode == SNAP_AI_MODE_BYPASS);
  assert(snap_ai_set_enabled(h,1) == SNAP_AI_OK);
  assert(snap_ai_set_profile(h,SNAP_AI_PROFILE_QUALITY) == SNAP_AI_OK);
  assert(snap_ai_set_profile(h,static_cast<SnapAIProfile>(99)) == SNAP_AI_ERROR_INVALID_ARGUMENT);
  SnapAIStats stats{}; stats.struct_size = sizeof(stats);
  assert(snap_ai_get_stats(h,&stats) == SNAP_AI_OK); assert(stats.frames_seen >= 23);
  snap_ai_destroy(h);
  c.min_flow_scale=1.2f; c.max_flow_scale=.8f; assert(!snap_ai_create(&c));
  SnapAIHandle* defaults = snap_ai_create(nullptr); assert(defaults); snap_ai_destroy(defaults);
  std::puts("Snap AI Engine tests passed");
}
