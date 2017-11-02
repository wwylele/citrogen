#pragma once

#include "core/container_backend/container.h"
#include "core/secret_backend/secret_database.h"

namespace CB {

class Ncsd : public FileContainer {
public:
  Ncsd(FB::FilePtr file);

private:
  SB::SecretContext secrets;
};

} // namespace CB