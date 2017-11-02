#pragma once

#include "core/container_backend/container.h"
#include "core/secret_backend/secret_database.h"

namespace CB {

class Exheader : public FileContainer {
public:
  Exheader(FB::FilePtr file);

private:
  SB::SecretContext secrets;
  u8 SciFlags();
};

} // namespace CB