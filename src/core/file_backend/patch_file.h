#pragma once

#include "file.h"

namespace FB {

class PatchFile : public File {
public:
  PatchFile(FilePtr base_, FilePtr patch_, std::size_t patch_offset_);

  std::size_t GetSize() override;
  byte_seq Read(std::size_t pos, std::size_t size) override;

private:
  FilePtr base;
  FilePtr patch;
  std::size_t patch_offset;
};

} // namespace FB