#include "snap_ai/frame_telemetry.h"
#include <cassert>
#include <cmath>
#include <cstdio>

using namespace snap_ai::telemetry;

static PresentEvent event(uint64_t id, uint64_t frame, uint64_t ms, Stream stream,
                          PresentApi api = PresentApi::Present, uint64_t gen = 1) {
  PresentEvent e{}; e.logical_present_id=id; e.frame_id=frame;
  e.monotonic_time_ns=ms*1000000; e.cpu_duration_ns=500000;
  e.swapchain_generation=gen; e.stream=stream; e.api=api; return e;
}
int main() {
  Recorder r(32);
  assert(r.record(event(1,1,10,Stream::Source)));
  assert(!r.record(event(1,1,10,Stream::Source,PresentApi::Present1)));
  assert(r.record(event(2,2,20,Stream::Generated)));
  assert(r.record(event(3,3,30,Stream::Source)));
  assert(r.record(event(4,4,40,Stream::Generated)));
  auto s=r.snapshot();
  assert(s.accepted_events==4 && s.duplicate_events==1);
  assert(s.source.presents==2 && s.generated.presents==2);
  assert(std::fabs(s.source.interval_median_ms-20.0)<0.001);
  assert(std::fabs(s.generated.interval_median_ms-20.0)<0.001);
  assert(std::fabs(s.output_interval_median_ms-10.0)<0.001);
  assert(!r.record(event(5,5,50,Stream::Source,PresentApi::Present,0)));
  assert(r.snapshot().stale_generation_events==1);
  r.reset(2);
  assert(r.record(event(1,1,100,Stream::Source,PresentApi::Vulkan,2)));
  assert(r.snapshot().source.presents==1);
  assert(!r.record(event(0,0,0,Stream::Source)));
  assert(r.snapshot().invalid_events==1);
  std::puts("Snap AI frame telemetry tests passed");
}
