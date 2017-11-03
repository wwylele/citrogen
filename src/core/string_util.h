#include "core/common_types.h"
#include <string>

template <bool upper_case> constexpr char DigitToHex(u8 value) {
  if (value < 10)
    return '0' + value;
  if constexpr (upper_case)
    return 'A' + value - 10;
  else
    return 'a' + value - 10;
}

template <bool upper_case, typename T> std::string IntToHex(T value) {
  std::string result;
  for (std::size_t i = 0; i < sizeof(T) * 2; ++i) {
    u8 digit = (u8)(value & 0xF);
    value >>= 4;
    result = DigitToHex<upper_case>(digit) + result;
  }
  return result;
}