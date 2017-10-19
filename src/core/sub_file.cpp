#include "core/sub_file.h"

namespace FB {

SubFile::SubFile(FilePtr parent, std::size_t offset, std::size_t file_size)
    : parent(std::move(parent)), offset(offset), file_size(file_size) {}

std::size_t SubFile::GetSize() { return file_size; }

std::vector<u8> SubFile::Read(std::size_t pos, std::size_t size) {
  if (pos >= file_size) {
    return {};
  }

  size = std::min(size, file_size - pos);
  return parent->Read(offset + pos, size);
}
} // namespace FB