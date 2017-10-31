#pragma once

#include "core/common_types.h"
#include <cryptopp/config.h>

inline CryptoPP::byte *CryptoPPBytes(byte_seq &bytes) {
  return reinterpret_cast<CryptoPP::byte *>(bytes.data());
}

inline const CryptoPP::byte *CryptoPPBytes(const byte_seq &bytes) {
  return reinterpret_cast<const CryptoPP::byte *>(bytes.data());
}
