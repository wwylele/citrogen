#include "core/secret_backend/movable_sed.h"
#include "core/file_backend/file.h"

namespace SB {

SecretDatabase FromMovableSed(FB::FilePtr file) {
  SecretDatabase secrets;
  secrets.Set(k_sec_key34_y, file->Read(0x110, 0x10));
  return secrets;
}

} // namespace SB