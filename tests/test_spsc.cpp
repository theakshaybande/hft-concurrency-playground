#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <thread>

#include "hft/spsc_ring_buffer.hpp"

int main() {
  constexpr std::size_t kIters = 1'000'000;
  hft::SpscRingBuffer rb(1024);

  std::atomic<bool> producer_done{false};
  std::atomic<bool> consumer_ok{true};

  std::thread producer([&] {
    for (std::size_t i = 0; i < kIters; ++i) {
      while (!rb.try_push(static_cast<std::uint64_t>(i))) {
      }
    }
    producer_done.store(true, std::memory_order_release);
  });

  std::thread consumer([&] {
    std::size_t expected = 0;
    std::uint64_t value = 0;
    while (!producer_done.load(std::memory_order_acquire) || expected < kIters) {
      if (rb.try_pop(value)) {
        if (value != expected) {
          consumer_ok.store(false, std::memory_order_release);
          return;
        }
        ++expected;
      }
    }
  });

  producer.join();
  consumer.join();

  if (!consumer_ok.load(std::memory_order_acquire)) {
    std::cerr << "SPSC test failed: FIFO order mismatch\n";
    return 1;
  }

  std::cout << "SPSC test passed\n";
  return 0;
}
