#include "snap_ai/frame_telemetry.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <mutex>
#include <vector>

namespace snap_ai::telemetry {
namespace {
constexpr size_t kRecentIds = 256;
double percentile(std::vector<double> values, double p) {
  if (values.empty()) return 0.0;
  std::sort(values.begin(), values.end());
  const size_t index = static_cast<size_t>(std::ceil(p * values.size())) - 1;
  return values[std::min(index, values.size() - 1)];
}
struct Sample {
  uint64_t time_ns = 0;
  uint64_t cpu_ns = 0;
  Stream stream = Stream::Source;
};
}

struct Recorder::Impl {
  explicit Impl(uint32_t n) : capacity(std::max<uint32_t>(n, 8)), samples(capacity) {}
  mutable std::mutex mutex;
  size_t capacity;
  std::vector<Sample> samples;
  size_t count = 0, next = 0;
  std::array<uint64_t, kRecentIds> recent{};
  size_t recent_next = 0, recent_count = 0;
  uint64_t generation = 0;
  Snapshot counters{};

  bool duplicate(uint64_t id) const {
    for (size_t i = 0; i < recent_count; ++i)
      if (recent[i] == id) return true;
    return false;
  }
  void push(const PresentEvent& e) {
    samples[next] = {e.monotonic_time_ns, e.cpu_duration_ns, e.stream};
    next = (next + 1) % capacity;
    count = std::min(count + 1, capacity);
    recent[recent_next] = e.logical_present_id;
    recent_next = (recent_next + 1) % kRecentIds;
    recent_count = std::min(recent_count + 1, kRecentIds);
  }
};

Recorder::Recorder(uint32_t capacity) : impl_(new Impl(capacity)) {}
Recorder::~Recorder() = default;
Recorder::Recorder(Recorder&&) noexcept = default;
Recorder& Recorder::operator=(Recorder&&) noexcept = default;

bool Recorder::record(const PresentEvent& e) {
  if (!impl_) return false;
  std::lock_guard<std::mutex> lock(impl_->mutex);
  if (!e.logical_present_id || !e.frame_id || !e.monotonic_time_ns ||
      (e.stream != Stream::Source && e.stream != Stream::Generated) ||
      (e.api != PresentApi::Present && e.api != PresentApi::Present1 &&
       e.api != PresentApi::Vulkan)) {
    ++impl_->counters.invalid_events; return false;
  }
  if (e.swapchain_generation < impl_->generation) {
    ++impl_->counters.stale_generation_events; return false;
  }
  if (e.swapchain_generation > impl_->generation) {
    impl_->generation = e.swapchain_generation;
    impl_->count = impl_->next = impl_->recent_count = impl_->recent_next = 0;
  }
  if (impl_->duplicate(e.logical_present_id)) {
    ++impl_->counters.duplicate_events; return false;
  }
  impl_->push(e);
  ++impl_->counters.accepted_events;
  impl_->counters.current_generation = impl_->generation;
  return true;
}

void Recorder::reset(uint64_t generation) {
  if (!impl_) return;
  std::lock_guard<std::mutex> lock(impl_->mutex);
  impl_->generation = generation;
  impl_->count = impl_->next = impl_->recent_count = impl_->recent_next = 0;
  impl_->counters.current_generation = generation;
}

Snapshot Recorder::snapshot() const {
  Snapshot out{};
  if (!impl_) return out;
  std::lock_guard<std::mutex> lock(impl_->mutex);
  out = impl_->counters;
  out.current_generation = impl_->generation;
  std::vector<double> all_intervals, source_intervals, generated_intervals;
  std::vector<double> source_cpu, generated_cpu;
  uint64_t last_all = 0, last_source = 0, last_generated = 0;
  uint64_t source_count = 0, generated_count = 0;
  const size_t start = (impl_->next + impl_->capacity - impl_->count) % impl_->capacity;
  for (size_t i = 0; i < impl_->count; ++i) {
    const Sample& s = impl_->samples[(start + i) % impl_->capacity];
    if (last_all && s.time_ns > last_all)
      all_intervals.push_back(static_cast<double>(s.time_ns - last_all) / 1.0e6);
    last_all = s.time_ns;
    if (s.stream == Stream::Source) {
      ++source_count;
      if (last_source && s.time_ns > last_source)
        source_intervals.push_back(static_cast<double>(s.time_ns - last_source) / 1.0e6);
      last_source = s.time_ns;
      if (s.cpu_ns) source_cpu.push_back(static_cast<double>(s.cpu_ns) / 1.0e6);
    } else {
      ++generated_count;
      if (last_generated && s.time_ns > last_generated)
        generated_intervals.push_back(static_cast<double>(s.time_ns - last_generated) / 1.0e6);
      last_generated = s.time_ns;
      if (s.cpu_ns) generated_cpu.push_back(static_cast<double>(s.cpu_ns) / 1.0e6);
    }
  }
  auto fill = [](StreamStats& s, uint64_t n, const std::vector<double>& intervals,
                 const std::vector<double>& cpu) {
    s.presents = n;
    s.interval_median_ms = percentile(intervals, .50);
    s.interval_p95_ms = percentile(intervals, .95);
    s.interval_p99_ms = percentile(intervals, .99);
    s.present_cpu_median_ms = percentile(cpu, .50);
    s.present_cpu_p95_ms = percentile(cpu, .95);
  };
  fill(out.source, source_count, source_intervals, source_cpu);
  fill(out.generated, generated_count, generated_intervals, generated_cpu);
  out.output_interval_median_ms = percentile(all_intervals, .50);
  out.output_interval_p95_ms = percentile(all_intervals, .95);
  out.output_interval_p99_ms = percentile(all_intervals, .99);
  return out;
}
} // namespace snap_ai::telemetry
