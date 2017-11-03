#include "core/secret_backend/bootrom.h"
#include "core/file_backend/file.h"

namespace SB {

SecretDatabase FromBoot9(FB::FilePtr file) {
  SecretDatabase secrets;
  secrets.Set(k_sec_key2C_x, file->Read(0xD9D0, 0x10));
  secrets.Set(k_sec_key34_x, file->Read(0xD9F0, 0x10));
  return secrets;
}

SecretDatabase FromBoot11(FB::FilePtr file) {
  SecretDatabase secrets;
  secrets.Set(k_sec_pubkey_ncsd_cfa, file->Read(0xC878, 0x100));
  secrets.Set(k_sec_pubkey_exheader, file->Read(0xC978, 0x100));
  return secrets;
}

} // namespace SB