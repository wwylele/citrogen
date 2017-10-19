#pragma once

#include "core/container.h"

namespace CB {

class Rsa : public ContainerHelper {
public:
  Rsa(FB::FilePtr data, FB::FilePtr signature, FB::FilePtr public_key);
  std::any Value() override;

private:
  FB::FilePtr data;
  FB::FilePtr signature;
  FB::FilePtr public_key;
};

} // namespace CB