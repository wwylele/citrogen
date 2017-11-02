#pragma once

#include "core/container_backend/container.h"

namespace CB {

class DiskDirectory : public ContainerHelper {
public:
  DiskDirectory(const std::string &path);

private:
};

} // namespace CB