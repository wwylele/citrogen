#include "patch_file.h"
#include <algorithm>

namespace FB {

PatchFile::PatchFile(FilePtr base_, FilePtr patch_, std::size_t patch_offset_)
    : base(std::move(base_)), patch(std::move(patch_)),
      patch_offset(patch_offset_) {}

std::size_t PatchFile::GetSize() {
  return std::max(base->GetSize(), patch_offset + patch->GetSize());
}

byte_seq PatchFile::Read(std::size_t pos, std::size_t read_size) {
  std::size_t base_size = base->GetSize();
  byte_seq result;
  if (pos < patch_offset) {
    std::size_t pre_size =
        std::min({read_size, patch_offset - pos, base_size - pos});
    result += base->Read(pos, pre_size);
    read_size -= pre_size;
    pos += pre_size;
  }

  std::size_t patch_size = patch->GetSize();
  std::size_t patch_end = patch_size + patch_offset;

  if (read_size != 0 && pos < patch_end) {
    std::size_t in_size = std::min(patch_end - pos, read_size);
    result += patch->Read(pos - patch_offset, in_size);
    read_size -= in_size;
    pos += in_size;
  }

  if (read_size != 0) {
    result += base->Read(pos, read_size);
  }

  return result;
}
} // namespace FB