#pragma once

#include "core/common_types.h"

constexpr std::size_t AES_BLOCK_SIZE = 16;

using AESKey = std::array<byte, AES_BLOCK_SIZE>;

AESKey ScrambleKey(const AESKey &x, const AESKey &y, const AESKey &c);
