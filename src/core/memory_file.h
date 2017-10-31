#pragma once

#include "core/file.h"
#include <vector>

namespace FB {

class MemoryFile : public File, public byte_seq {
public:
  using byte_seq::vector;
  MemoryFile() = default;
  MemoryFile(const byte_seq &);
  MemoryFile(byte_seq &&);

  std::size_t GetSize() override;
  byte_seq Read(std::size_t pos, std::size_t size) override;
};

} // namespace FB