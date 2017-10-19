#pragma once

#include "core/file.h"

namespace FB {

class AesCbcFile : public File {
public:
  AesCbcFile(FilePtr parent, FilePtr key, FilePtr iv);

  std::size_t GetSize() override;
  std::vector<u8> Read(std::size_t pos, std::size_t size) override;

private:
  FilePtr parent;
  FilePtr key;
  FilePtr iv;
};

} // namespace FB