#pragma once

#include "core/container_backend/container.h"

namespace CB {

class Cia : public FileContainer {
public:
  Cia(FB::FilePtr file);

private:
  FB::FilePtr metadata, content;
  std::vector<FB::FilePtr> sub_contents;
};

} // namespace CB