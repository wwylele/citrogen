#include "core/smdh.h"

namespace CB {

// 8x8 Z-Order coordinate from 2D coordinates
static inline std::size_t MortonInterleave(std::size_t x, std::size_t y) {
  static const std::size_t xlut[] = {0x00, 0x01, 0x04, 0x05,
                                     0x10, 0x11, 0x14, 0x15};
  static const std::size_t ylut[] = {0x00, 0x02, 0x08, 0x0a,
                                     0x20, 0x22, 0x28, 0x2a};
  return xlut[x % 8] + ylut[y % 8];
}

/**
 * Calculates the offset of the position of the pixel in Morton order
 */
static inline std::size_t GetMortonOffset(std::size_t x, std::size_t y) {
  // Images are split into 8x8 tiles. Each tile is composed of four 4x4 subtiles
  // each of which is composed of four 2x2 subtiles each of which is composed of
  // four texels. Each structure is embedded into the next-bigger one in a
  // diagonal pattern, e.g. texels are laid out in a 2x2 subtile like this: 2 3
  // 0 1
  //
  // The full 8x8 tile has the texels arranged like this:
  //
  // 42 43 46 47 58 59 62 63
  // 40 41 44 45 56 57 60 61
  // 34 35 38 39 50 51 54 55
  // 32 33 36 37 48 49 52 53
  // 10 11 14 15 26 27 30 31
  // 08 09 12 13 24 25 28 29
  // 02 03 06 07 18 19 22 23
  // 00 01 04 05 16 17 20 21
  //
  // This pattern is what's called Z-order curve, or Morton order.

  const std::size_t block_height = 8;
  const std::size_t coarse_x = x & ~7;

  const std::size_t i = MortonInterleave(x, y);

  const std::size_t offset = coarse_x * block_height;

  return i + offset;
}

template <std::size_t size, std::size_t offset>
std::array<u16, size * size> Smdh::GetIcon() {
  std::array<u16, size * size> result;
  for (std::size_t x = 0; x < size; ++x) {
    for (std::size_t y = 0; y < size; ++y) {
      std::size_t coarse_y = y & ~7;
      std::size_t pixel_offset =
          offset + (GetMortonOffset(x, y) + coarse_y * size) * 2;
      result[x + size * y] = file->Read<u16>(pixel_offset);
    }
  }
  return result;
};

Smdh::Smdh(FB::FilePtr file_) : FileContainer(std::move(file_)) {
  InstallList({
      {"IconLarge",
       [this]() {
         return std::make_shared<ConstContainer>(GetIcon<48, 0x24C0>());
       }},
      {"IconSmall",
       [this]() {
         return std::make_shared<ConstContainer>(GetIcon<24, 0x2040>());
       }},
  });
}

} // namespace CB