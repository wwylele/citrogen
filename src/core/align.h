#pragma once
#include <cstdlib>
#include <type_traits>

template <typename T> constexpr T AlignUp(T value, std::size_t size) {
  static_assert(std::is_unsigned<T>::value, "T must be an unsigned value.");
  return static_cast<T>(value + (size - value % size) % size);
}

template <typename T> constexpr T AlignDown(T value, std::size_t size) {
  static_assert(std::is_unsigned<T>::value, "T must be an unsigned value.");
  return static_cast<T>(value - value % size);
}