#pragma once

#include "container.h"

namespace CB {

class Romfs : public FileContainer {
public:
  Romfs(FB::FilePtr file_);

private:
  FB::FilePtr Level0File();
  FB::FilePtr Level1Or2File(const std::string &number, bool align_up);
  FB::FilePtr Level1File(bool align_up);
  FB::FilePtr Level2File(bool align_up);
  FB::FilePtr Level3File(bool align_up);
};

} // namespace CB