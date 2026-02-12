#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char** argv) {
  const std::size_t rounds = (argc > 1) ? static_cast<std::size_t>(std::stoull(argv[1])) : 3'000'000;

  std::atomic<std::uint64_t> payload{0};
  std::atomic<bool> ready{false};
  std::atomic<bool> stop{false};
  std::uint64_t observed_sum = 0;

  std::thread consumer([&] {
    while (!stop.load(std::memory_order_relaxed)) {
      if (ready.load(std::memory_order_acquire)) {
        observed_sum += payload.load(std::memory_order_relaxed);
        ready.store(false, std::memory_order_relaxed);
      }
    }
  });

  const auto start = std::chrono::steady_clock::now();
  for (std::size_t i = 1; i <= rounds; ++i) {
    while (ready.load(std::memory_order_relaxed)) {
    }
    payload.store(static_cast<std::uint64_t>(i), std::memory_order_relaxed);
    ready.store(true, std::memory_order_release);
  }
  while (ready.load(std::memory_order_relaxed)) {
  }
  stop.store(true, std::memory_order_relaxed);
  consumer.join();
  const auto end = std::chrono::steady_clock::now();

  const std::uint64_t expected = (static_cast<std::uint64_t>(rounds) * (static_cast<std::uint64_t>(rounds) + 1)) / 2;
  const double seconds = std::chrono::duration<double>(end - start).count();

  std::cout << "memory_order_demo\n";
  std::cout << "rounds: " << rounds << "\n";
  std::cout << "observed sum: " << observed_sum << "\n";
  std::cout << "expected sum: " << expected << "\n";
  std::cout << "match: " << (observed_sum == expected ? "yes" : "no") << "\n";
  std::cout << "elapsed: " << seconds << " s\n";
  return (observed_sum == expected) ? 0 : 1;
}
