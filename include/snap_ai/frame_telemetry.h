#ifndef SNAP_AI_FRAME_TELEMETRY_H
#define SNAP_AI_FRAME_TELEMETRY_H

#include "snap_ai/snap_ai.h"
#include <cstdint>
#include <memory>

namespace snap_ai::telemetry {

enum class Stream : uint8_t { Source = 0, Generated = 1 };
enum class PresentApi : uint8_t { Present = 0, Present1 = 1, Vulkan = 2 };

struct PresentEvent {
  uint64_t logical_present_id = 0;
  uint64_t frame_id = 0;
  uint64_t monotonic_time_ns = 0;
  uint64_t cpu_duration_ns = 0;
  uint64_t swapchain_generation = 0;
  Stream stream = Stream::Source;
  PresentApi api = PresentApi::Present;
};

struct StreamStats {
  uint64_t presents = 0;
  double interval_median_ms = 0.0;
  double interval_p95_ms = 0.0;
  double interval_p99_ms = 0.0;
  double present_cpu_median_ms = 0.0;
  double present_cpu_p95_ms = 0.0;
};

struct Snapshot {
  uint64_t accepted_events = 0;
  uint64_t duplicate_events = 0;
  uint64_t stale_generation_events = 0;
  uint64_t invalid_events = 0;
  uint64_t current_generation = 0;
  StreamStats source{};
  StreamStats generated{};
  double output_interval_median_ms = 0.0;
  double output_interval_p95_ms = 0.0;
  double output_interval_p99_ms = 0.0;
};

class SNAP_AI_API Recorder {
public:
  explicit Recorder(uint32_t capacity = 256);
  ~Recorder();
  Recorder(Recorder&&) noexcept;
  Recorder& operator=(Recorder&&) noexcept;
  Recorder(const Recorder&) = delete;
  Recorder& operator=(const Recorder&) = delete;

  bool record(const PresentEvent& event);
  void reset(uint64_t new_swapchain_generation);
  Snapshot snapshot() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace snap_ai::telemetry
#endif
