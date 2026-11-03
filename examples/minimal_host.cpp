#include <snap_ai/snap_ai.h>
#include <cstdio>
int main() {
  SnapAIConfig cfg{}; snap_ai_default_config(&cfg);
  cfg.profile = SNAP_AI_PROFILE_BALANCED;
  SnapAIHandle* ai = snap_ai_create(&cfg); if (!ai) return 1;
  for (uint64_t i=1; i<=16; ++i) {
    SnapAIFrameMetrics m{}; m.struct_size=sizeof(m); m.frame_id=i;
    m.frame_time_ms=33.3f; m.gpu_time_ms=3.5f; m.motion_magnitude=.18f;
    m.motion_complexity=.22f; m.flow_confidence=.94f; m.gpu_pressure=.55f;
    m.artifact_risk=.05f; m.telemetry_valid=1;
    SnapAIDecision d{}; d.struct_size=sizeof(d);
    if (snap_ai_update(ai,&m,&d) != SNAP_AI_OK) { snap_ai_destroy(ai); return 2; }
    std::printf("frame=%llu mode=%d flow_scale=%.2f radius=%.2f passes=%u\n",
      static_cast<unsigned long long>(d.frame_id), static_cast<int>(d.mode),
      d.flow_scale, d.search_radius_scale, d.refinement_passes);
  }
  snap_ai_destroy(ai); return 0;
}
