#include "core/memory_file.h"

namespace FB {

MemoryFile::MemoryFile(const std::vector<u8> &v) : std::vector<u8>(v) {}

MemoryFile::MemoryFile(std::vector<u8> &&v) : std::vector<u8>(std::move(v)) {}

std::size_t MemoryFile::GetSize() { return size(); }

std::vector<u8> MemoryFile::Read(std::size_t pos, std::size_t read_size) {
  if (pos >= size()) {
    return {};
  }

  read_size = std::min(read_size, size() - pos);
  return std::vector<u8>(begin() + pos, begin() + pos + read_size);
}
} // namespace FB