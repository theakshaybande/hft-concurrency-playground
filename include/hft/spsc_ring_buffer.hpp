#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "hft/cacheline.hpp"

namespace hft {

class SpscRingBuffer {
 public:
  explicit SpscRingBuffer(std::size_t requested_capacity)
      : capacity_(next_power_of_two(requested_capacity)),
        mask_(capacity_ - 1),
        data_(capacity_, 0) {}

  SpscRingBuffer(const SpscRingBuffer&) = delete;
  SpscRingBuffer& operator=(const SpscRingBuffer&) = delete;

  bool try_push(std::uint64_t value) noexcept {
    const std::size_t head = head_.value.load(std::memory_order_relaxed);
    const std::size_t tail = tail_.value.load(std::memory_order_acquire);
    if ((head - tail) == capacity_) {
      return false;
    }
    data_[head & mask_] = value;
    head_.value.store(head + 1, std::memory_order_release);
    return true;
  }

  bool try_pop(std::uint64_t& out) noexcept {
    const std::size_t tail = tail_.value.load(std::memory_order_relaxed);
    const std::size_t head = head_.value.load(std::memory_order_acquire);
    if (head == tail) {
      return false;
    }
    out = data_[tail & mask_];
    tail_.value.store(tail + 1, std::memory_order_release);
    return true;
  }

  std::size_t capacity() const noexcept { return capacity_; }

 private:
  static std::size_t next_power_of_two(std::size_t v) noexcept {
    if (v < 2) {
      return 2;
    }
    --v;
    for (std::size_t shift = 1; shift < sizeof(std::size_t) * 8; shift <<= 1) {
      v |= v >> shift;
    }
    return v + 1;
  }

  const std::size_t capacity_;
  const std::size_t mask_;
  std::vector<std::uint64_t> data_;
  CacheLinePadded<std::atomic<std::size_t>> head_{};
  CacheLinePadded<std::atomic<std::size_t>> tail_{};
};

}  // namespace hft
