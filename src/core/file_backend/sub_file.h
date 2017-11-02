#pragma once

#include "file.h"

namespace FB {

class SubFile : public File {
public:
  SubFile(FilePtr parent, std::size_t offset, std::size_t file_size);

  std::size_t GetSize() override;
  byte_seq Read(std::size_t pos, std::size_t size) override;

private:
  FilePtr parent;
  std::size_t offset;
  std::size_t file_size;
};

} // namespace FB