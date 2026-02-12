#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>

#include "hft/cacheline.hpp"

namespace {

struct SharedCounters {
  std::atomic<std::uint64_t> a{0};
  std::atomic<std::uint64_t> b{0};
};

struct PaddedCounters {
  hft::CacheLinePadded<std::atomic<std::uint64_t>> a;
  hft::CacheLinePadded<std::atomic<std::uint64_t>> b;
};

template <typename T>
double run(T& counters, std::size_t iters) {
  auto start = std::chrono::steady_clock::now();
  std::thread t1([&] {
    for (std::size_t i = 0; i < iters; ++i) {
      counters.a.value.fetch_add(1, std::memory_order_relaxed);
    }
  });
  std::thread t2([&] {
    for (std::size_t i = 0; i < iters; ++i) {
      counters.b.value.fetch_add(1, std::memory_order_relaxed);
    }
  });
  t1.join();
  t2.join();
  auto end = std::chrono::steady_clock::now();
  return std::chrono::duration<double>(end - start).count();
}

template <>
double run(SharedCounters& counters, std::size_t iters) {
  auto start = std::chrono::steady_clock::now();
  std::thread t1([&] {
    for (std::size_t i = 0; i < iters; ++i) {
      counters.a.fetch_add(1, std::memory_order_relaxed);
    }
  });
  std::thread t2([&] {
    for (std::size_t i = 0; i < iters; ++i) {
      counters.b.fetch_add(1, std::memory_order_relaxed);
    }
  });
  t1.join();
  t2.join();
  auto end = std::chrono::steady_clock::now();
  return std::chrono::duration<double>(end - start).count();
}

}  // namespace

int main(int argc, char** argv) {
  const std::size_t iterations = (argc > 1) ? static_cast<std::size_t>(std::stoull(argv[1])) : 50'000'000;

  SharedCounters shared;
  PaddedCounters padded;
  const double shared_s = run(shared, iterations);
  const double padded_s = run(padded, iterations);

  std::cout << "false_sharing_demo\n";
  std::cout << "iterations per thread: " << iterations << "\n";
  std::cout << "shared counters: " << shared_s << " s\n";
  std::cout << "padded counters: " << padded_s << " s\n";
  if (padded_s > 0.0) {
    std::cout << "speedup: " << (shared_s / padded_s) << "x\n";
  }
  return 0;
}
