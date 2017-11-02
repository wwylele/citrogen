#include "core/file_backend/memory_file.h"

namespace FB {

MemoryFile::MemoryFile(const byte_seq &v) : byte_seq(v) {}

MemoryFile::MemoryFile(byte_seq &&v) : byte_seq(std::move(v)) {}

std::size_t MemoryFile::GetSize() { return size(); }

byte_seq MemoryFile::Read(std::size_t pos, std::size_t read_size) {
  if (pos >= size()) {
    return {};
  }

  read_size = std::min(read_size, size() - pos);
  return byte_seq(begin() + pos, begin() + pos + read_size);
}
} // namespace FB