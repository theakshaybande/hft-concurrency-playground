#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include "hft/histogram.hpp"

int main(int argc, char** argv) {
  const std::size_t iterations = (argc > 1) ? static_cast<std::size_t>(std::stoull(argv[1])) : 2'000'000;

  hft::Histogram hist(50, 20'000);
  std::deque<std::uint64_t> q;
  std::mutex m;
  std::condition_variable cv;
  std::atomic<bool> ready{false};
  std::atomic<bool> done{false};

  std::thread consumer([&] {
    while (!ready.load(std::memory_order_acquire)) {
    }
    std::size_t consumed = 0;
    while (consumed < iterations) {
      std::unique_lock<std::mutex> lock(m);
      cv.wait(lock, [&] { return !q.empty(); });
      q.pop_front();
      ++consumed;
    }
    done.store(true, std::memory_order_release);
  });

  ready.store(true, std::memory_order_release);
  const auto start = std::chrono::steady_clock::now();
  for (std::size_t i = 0; i < iterations; ++i) {
    const auto t0 = std::chrono::steady_clock::now();
    {
      std::lock_guard<std::mutex> lock(m);
      q.push_back(static_cast<std::uint64_t>(i));
    }
    cv.notify_one();
    const auto t1 = std::chrono::steady_clock::now();
    hist.observe(static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()));
  }
  while (!done.load(std::memory_order_acquire)) {
  }
  const auto end = std::chrono::steady_clock::now();
  consumer.join();

  const double seconds = std::chrono::duration<double>(end - start).count();
  const double throughput_mps = static_cast<double>(iterations) / seconds / 1'000'000.0;

  std::cout << "bench_mutex_queue\n";
  std::cout << "iterations: " << iterations << "\n";
  std::cout << "throughput: " << throughput_mps << " Mops/s\n";
  std::cout << "p50 push section (ns): " << hist.percentile(0.50) << "\n";
  std::cout << "p99 push section (ns): " << hist.percentile(0.99) << "\n";
  return 0;
}
