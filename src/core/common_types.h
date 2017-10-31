#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#if defined(_MSC_VER)
#include <cstdlib>
#elif defined(__linux__)
#include <byteswap.h>
#elif defined(__Bitrig__) || defined(__DragonFly__) || defined(__FreeBSD__) || \
    defined(__NetBSD__) || defined(__OpenBSD__)
#include <sys/endian.h>
#endif

using u8 = std::uint8_t;   ///< 8-bit unsigned byte
using u16 = std::uint16_t; ///< 16-bit unsigned short
using u32 = std::uint32_t; ///< 32-bit unsigned word
using u64 = std::uint64_t; ///< 64-bit unsigned int

using s8 = std::int8_t;   ///< 8-bit signed byte
using s16 = std::int16_t; ///< 16-bit signed short
using s32 = std::int32_t; ///< 32-bit signed word
using s64 = std::int64_t; ///< 64-bit signed int

using byte = std::byte;
using byte_seq = std::vector<byte>;

#ifdef _MSC_VER
inline u16 swap16(u16 _data) { return _byteswap_ushort(_data); }
inline u32 swap32(u32 _data) { return _byteswap_ulong(_data); }
inline u64 swap64(u64 _data) { return _byteswap_uint64(_data); }
#elif _M_ARM
inline u16 swap16(u16 _data) {
  u32 data = _data;
  __asm__("rev16 %0, %1\n" : "=l"(data) : "l"(data));
  return (u16)data;
}
inline u32 swap32(u32 _data) {
  __asm__("rev %0, %1\n" : "=l"(_data) : "l"(_data));
  return _data;
}
inline u64 swap64(u64 _data) {
  return ((u64)swap32(_data) << 32) | swap32(_data >> 32);
}
#elif __linux__
inline u16 swap16(u16 _data) { return bswap_16(_data); }
inline u32 swap32(u32 _data) { return bswap_32(_data); }
inline u64 swap64(u64 _data) { return bswap_64(_data); }
#elif __APPLE__
inline __attribute__((always_inline)) u16 swap16(u16 _data) {
  return (_data >> 8) | (_data << 8);
}
inline __attribute__((always_inline)) u32 swap32(u32 _data) {
  return __builtin_bswap32(_data);
}
inline __attribute__((always_inline)) u64 swap64(u64 _data) {
  return __builtin_bswap64(_data);
}
#elif defined(__Bitrig__) || defined(__OpenBSD__)
// swap16, swap32, swap64 are left as is
#elif defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__)
inline u16 swap16(u16 _data) { return bswap16(_data); }
inline u32 swap32(u32 _data) { return bswap32(_data); }
inline u64 swap64(u64 _data) { return bswap64(_data); }
#else
// Slow generic implementation.
inline u16 swap16(u16 data) { return (data >> 8) | (data << 8); }
inline u32 swap32(u32 data) {
  return (swap16(data) << 16) | swap16(data >> 16);
}
inline u64 swap64(u64 data) {
  return ((u64)swap32(data) << 32) | swap32(data >> 32);
}
#endif

template <typename T> T swap(T data);

template <> inline u16 swap<u16>(u16 data) { return swap16(data); }

template <> inline u32 swap<u32>(u32 data) { return swap32(data); }

template <> inline u64 swap<u64>(u64 data) { return swap64(data); }

using magic_t = std::array<char, 4>;
