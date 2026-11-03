#ifdef NDEBUG
#undef NDEBUG
#endif
#include "snap_frame/snap_frame.h"
#include <cassert>
#include <cstdint>
#include <cstring>

struct Fake { uint32_t calls=0, resets=0, updates=0; };
static SnapFrameStatus init(void*, const SnapFrameEngineConfig*) { return SNAP_FRAME_OK; }
static void shutdown(void*) {}
static SnapFrameStatus update(void* u, const SnapFrameEngineConfig*) {
  static_cast<Fake*>(u)->updates++; return SNAP_FRAME_OK;
}
static void reset(void* u, uint32_t) { static_cast<Fake*>(u)->resets++; }
static SnapFrameStatus process(void* u, const SnapFrameFrameInput*, SnapFrameFrameOutput* o) {
  static_cast<Fake*>(u)->calls++; o->generated_frames=1; o->flow_gpu_ms=2.5f; return SNAP_FRAME_OK;
}
int main() {
  SnapFrameEngineConfig c; snap_frame_default_config(&c);
  Fake f;
  SnapFrameBackendV1 b{}; b.struct_size=sizeof(b); b.api_version=SNAP_FRAME_API_VERSION;
  b.user_data=&f; b.initialize=init; b.shutdown=shutdown; b.update_config=update;
  b.reset_history=reset; b.process_frame=process;
  SnapFrameEngine* e=snap_frame_create(&c,&b); assert(e);
  SnapFrameFrameInput in{}; in.struct_size=sizeof(in); in.frame_id=1; in.width=1280; in.height=720;
  SnapFrameFrameOutput out{}; out.struct_size=sizeof(out);
  assert(snap_frame_process(e,&in,&out)==SNAP_FRAME_BYPASSED);
  assert(f.calls==0);
  assert(snap_frame_set_enabled(e,1)==SNAP_FRAME_OK);
  in.frame_id=2;
  assert(snap_frame_process(e,&in,&out)==SNAP_FRAME_OK);
  assert(f.calls==1 && out.generated_frames==1);
  assert(snap_frame_set_quality(e,SNAP_FRAME_QUALITY_PERFORMANCE)==SNAP_FRAME_OK);
  assert(f.updates==2);
  in.frame_id=3; in.scene_cut=1;
  assert(snap_frame_process(e,&in,&out)==SNAP_FRAME_OK);
  assert(f.resets==2); // enable transition + scene cut
  in.frame_id=3;
  assert(snap_frame_process(e,&in,&out)==SNAP_FRAME_ERROR_FRAME_ORDER);
  SnapFrameEngineStats stats{}; stats.struct_size=sizeof(stats);
  assert(snap_frame_get_stats(e,&stats)==SNAP_FRAME_OK);
  assert(stats.frames_seen==4 && stats.frames_processed==2 && stats.generated_frames==2);
  snap_frame_destroy(e);
  return 0;
}
