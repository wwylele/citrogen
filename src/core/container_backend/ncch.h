#pragma once

#include "core/container_backend/container.h"
#include "core/secret_backend/secret_database.h"

namespace CB {

class Ncch : public FileContainer {
public:
  Ncch(FB::FilePtr file);

private:
  SB::SecretContext secrets;

  enum class SeedStatus {
    NoNeed,
    NotFound,
    NotCorrect,
    Found,
  } seed_status;
  byte_seq seed;

  void CheckForceNoCrypto();
  bool force_no_crypto = false;

  enum class IvType : u8 {
    Exheader = 1,
    Exefs = 2,
    Romfs = 3,
  };

  FB::FilePtr PatchedHeader();

  bool IsDecrypted();
  void InitSeed();
  FB::FilePtr KeyY();
  FB::FilePtr PrimaryNormalKey();
  FB::FilePtr SecondaryNormalKey();
  std::string PrimaryNormalKeyError();
  std::string SecondaryNormalKeyError();

  FB::FilePtr CryptoIv(IvType type);
  FB::FilePtr ExheaderFile();
  std::string ExheaderError();
  FB::FilePtr RawExefsFile();
  FB::FilePtr PrimaryExefsFile();
  FB::FilePtr SecondaryExefsFile();
  std::string ExefsError();
  FB::FilePtr RawRomfsFile();
  FB::FilePtr RomfsFile();
  std::string RomfsError();

  u8 ContentType();
  u8 ContentType2();
};

} // namespace CB