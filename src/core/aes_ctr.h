#pragma once

#include "core/file.h"

namespace FB {

class AesCtrFile : public File {
public:
  AesCtrFile(FilePtr parent, FilePtr key, FilePtr iv);

  std::size_t GetSize() override;
  std::vector<u8> Read(std::size_t pos, std::size_t size) override;

private:
  FilePtr parent;
  FilePtr key;
  FilePtr iv;
};

} // namespace FB