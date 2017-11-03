#pragma once

#include "core/container_backend/container.h"
#include "core/secret_backend/secret_database.h"

namespace CB {

class SdProtected : public ContainerHelper {
public:
  SdProtected(ContainerPtr sd_root);

private:
  SB::SecretContext secrets;
};

} // namespace CB