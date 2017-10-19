#pragma once

#include "core/file.h"
#include <vector>

namespace FB {

class MemoryFile : public File, public std::vector<u8> {
public:
  using std::vector<u8>::vector;
  MemoryFile() = default;
  MemoryFile(const std::vector<u8> &);
  MemoryFile(std::vector<u8> &&);

  std::size_t GetSize() override;
  std::vector<u8> Read(std::size_t pos, std::size_t size) override;
};

} // namespace FB