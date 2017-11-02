#pragma once

#include "core/container_backend/container.h"

namespace CB {

class Smdh : public FileContainer {
public:
  Smdh(FB::FilePtr file);

private:
  template <std::size_t size, std::size_t offset>
  std::array<u16, size * size> GetIcon();
};

} // namespace CB