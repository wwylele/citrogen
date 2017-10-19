#pragma once

#include "core/container.h"
#include "core/secret_database.h"

namespace CB {

class Exheader : public FileContainer {
public:
  Exheader(FB::FilePtr file);

private:
  SB::SecretContext secrets;
  u8 SciFlags();
};

} // namespace CB