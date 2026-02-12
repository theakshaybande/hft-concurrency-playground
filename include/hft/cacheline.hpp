#pragma once

#include <cstddef>

namespace hft {

inline constexpr std::size_t kCacheLineSize = 64;

template <typename T>
struct alignas(kCacheLineSize) CacheLinePadded {
  T value{};
};

}  // namespace hft
