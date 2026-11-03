#include "snap_frame/snap_frame.h"
#include <cstdio>
struct HostBackend { unsigned processed=0; };
static SnapFrameStatus process(void* user,const SnapFrameFrameInput*,SnapFrameFrameOutput* out) {
  auto* host=static_cast<HostBackend*>(user);
  ++host->processed;
  out->generated_frames=1;
  out->flow_gpu_ms=1.25f; /* Mock value; not a GPU measurement. */
  return SNAP_FRAME_OK;
}
int main() {
  SnapFrameConfig config; snap_frame_default_config(&config); config.enabled=1;
  HostBackend host;
  SnapFrameBackendV1 backend{}; backend.struct_size=sizeof(backend);
  backend.api_version=SNAP_FRAME_API_VERSION; backend.user_data=&host;
  backend.process_frame=process;
  auto* engine=snap_frame_create(&config,&backend);
  if(!engine) return 1;
  SnapFrameFrameInput input{}; input.struct_size=sizeof(input);
  input.frame_id=1; input.width=1920; input.height=1080;
  SnapFrameFrameOutput output{}; output.struct_size=sizeof(output);
  const auto status=snap_frame_process(engine,&input,&output);
  std::printf("status=%d generated=%u backend_calls=%u (mock only)\n",
    int(status),output.generated_frames,host.processed);
  snap_frame_destroy(engine);
  return status==SNAP_FRAME_OK ? 0 : 2;
}
