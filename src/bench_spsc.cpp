#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>

#include "hft/histogram.hpp"
#include "hft/spsc_ring_buffer.hpp"

int main(int argc, char** argv) {
  const std::size_t iterations = (argc > 1) ? static_cast<std::size_t>(std::stoull(argv[1])) : 2'000'000;
  const std::size_t capacity = (argc > 2) ? static_cast<std::size_t>(std::stoull(argv[2])) : 65'536;

  hft::SpscRingBuffer rb(capacity);
  hft::Histogram hist(50, 20'000);

  std::atomic<bool> ready{false};
  std::atomic<bool> done{false};

  std::thread consumer([&] {
    while (!ready.load(std::memory_order_acquire)) {
    }
    std::uint64_t value = 0;
    std::size_t consumed = 0;
    while (consumed < iterations) {
      if (rb.try_pop(value)) {
        ++consumed;
      }
    }
    done.store(true, std::memory_order_release);
  });

  ready.store(true, std::memory_order_release);
  const auto start = std::chrono::steady_clock::now();
  for (std::size_t i = 0; i < iterations; ++i) {
    const auto t0 = std::chrono::steady_clock::now();
    while (!rb.try_push(static_cast<std::uint64_t>(i))) {
    }
    const auto t1 = std::chrono::steady_clock::now();
    hist.observe(static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()));
  }
  while (!done.load(std::memory_order_acquire)) {
  }
  const auto end = std::chrono::steady_clock::now();
  consumer.join();

  const double seconds = std::chrono::duration<double>(end - start).count();
  const double throughput_mps = static_cast<double>(iterations) / seconds / 1'000'000.0;

  std::cout << "bench_spsc\n";
  std::cout << "iterations: " << iterations << "\n";
  std::cout << "capacity:   " << rb.capacity() << "\n";
  std::cout << "throughput: " << throughput_mps << " Mops/s\n";
  std::cout << "p50 push wait (ns): " << hist.percentile(0.50) << "\n";
  std::cout << "p99 push wait (ns): " << hist.percentile(0.99) << "\n";
  return 0;
}
