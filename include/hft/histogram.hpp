#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace hft {

class Histogram {
 public:
  Histogram(std::size_t bucket_width_ns, std::size_t bucket_count)
      : bucket_width_ns_(bucket_width_ns),
        buckets_(bucket_count, 0),
        overflows_(0) {}

  void observe(std::uint64_t latency_ns) noexcept {
    const std::size_t index = static_cast<std::size_t>(latency_ns / bucket_width_ns_);
    if (index < buckets_.size()) {
      ++buckets_[index];
    } else {
      ++overflows_;
    }
  }

  std::uint64_t percentile(double p) const {
    std::uint64_t total = overflows_;
    for (auto c : buckets_) {
      total += c;
    }
    if (total == 0) {
      return 0;
    }

    const std::uint64_t rank = static_cast<std::uint64_t>(p * static_cast<double>(total - 1));
    std::uint64_t seen = 0;
    for (std::size_t i = 0; i < buckets_.size(); ++i) {
      seen += buckets_[i];
      if (seen > rank) {
        return static_cast<std::uint64_t>(i * bucket_width_ns_);
      }
    }
    return static_cast<std::uint64_t>(buckets_.size() * bucket_width_ns_);
  }

  std::uint64_t max_bucket_ns() const noexcept {
    return static_cast<std::uint64_t>(buckets_.size() * bucket_width_ns_);
  }

 private:
  std::size_t bucket_width_ns_;
  std::vector<std::uint64_t> buckets_;
  std::uint64_t overflows_;
};

}  // namespace hft
