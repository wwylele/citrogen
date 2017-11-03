#pragma once

#include "core/file_backend/file.h"
#include "core/secret_backend/secret_database.h"

namespace SB {

SecretDatabase FromMovableSed(FB::FilePtr file);

} // namespace SB