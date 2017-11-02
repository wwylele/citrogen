#pragma once

#include "core/file_backend/file.h"
#include "core/secret_backend/secret_database.h"

namespace SB {

SecretDatabase FromBoot9(FB::FilePtr file);
SecretDatabase FromBoot11(FB::FilePtr file);

} // namespace SB