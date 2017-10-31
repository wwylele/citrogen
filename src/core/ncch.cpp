#include "core/ncch.h"
#include "core/aes_ctr.h"
#include "core/aes_key.h"
#include "core/exefs.h"
#include "core/exheader.h"
#include "core/memory_file.h"
#include "core/romfs.h"
#include "core/rsa.h"
#include "core/sha.h"

namespace CB {

Ncch::Ncch(FB::FilePtr file) : FileContainer(std::move(file)) {
  InstallList({
      Field<magic_t>("Magic", 0x100),
      Field<u32>("ContentSize", 0x104),
      Field<u64>("PartitionId", 0x108),
      Field<u16>("MakerCode", 0x110),
      Field<u16>("Version", 0x112),

      Field<u64>("ProgramId", 0x118),

      Field<std::array<char, 0x10>>("ProductCode", 0x150),

      Field<u32>("ExheaderHashRegionSize", 0x180),

      Field<u8>("CryptoMethod", 0x18B),
      Field<u8>("Platform", 0x18C),

      Field<u8>("ContentTypeFlags", 0x18D),

      {"IsData",
       [this]() {
         return std::make_shared<ConstContainer>((ContentType() & 0x1) != 0);
       }},
      {"IsExecutable",
       [this]() {
         return std::make_shared<ConstContainer>((ContentType() & 0x2) != 0);
       }},
      {"ContentType",
       [this]() {
         return std::make_shared<ConstContainer>((u8)(ContentType() >> 2));
       }},

      Field<u8>("ContentType2", 0x18F),

      {"IsFixedKeyCrypto",
       [this]() {
         return std::make_shared<ConstContainer>((ContentType2() & 0x1) != 0);
       }},
      {"IsNoRomfsMount",
       [this]() {
         return std::make_shared<ConstContainer>((ContentType2() & 0x2) != 0);
       }},
      {"IsNoCrypto",
       [this]() {
         return std::make_shared<ConstContainer>((ContentType2() & 0x4) != 0);
       }},
      {"IsSeedCrypto",
       [this]() {
         return std::make_shared<ConstContainer>((ContentType2() & 0x20) != 0);
       }},

      Field<u32>("PlainRegionOffset", 0x190),
      Field<u32>("PlainRegionSize", 0x194),
      Field<u32>("LogoRegionOffset", 0x198),
      Field<u32>("LogoRegionSize", 0x19C),
      Field<u32>("ExefsOffset", 0x1A0),
      Field<u32>("ExefsSize", 0x1A4),
      Field<u32>("ExefsHashRegionSize", 0x1A8),

      Field<u32>("RomfsOffset", 0x1B0),
      Field<u32>("RomfsSize", 0x1B4),
      Field<u32>("RomfsHashRegionSize", 0x1B8),

  });

  CheckForceNoCrypto();

  InstallList({{"IsForceNoCrypto", [this]() {
                  return std::make_shared<ConstContainer>(force_no_crypto);
                }}});

  u32 exheader_hash_region_size = Open("ExheaderHashRegionSize")->ValueT<u32>();
  if (exheader_hash_region_size) {
    auto error = ExheaderError();
    InstallList({{"ExheaderError", [error]() {
                    return std::make_shared<ConstContainer>(error);
                  }}});
    if (error.empty()) {
      InstallList(
          {{"Exheader",
            [this]() { return std::make_shared<Exheader>(ExheaderFile()); }},
           {"ExheaderHash",
            [this, exheader_hash_region_size]() {
              return std::make_shared<Sha>(
                  std::make_shared<FB::SubFile>(ExheaderFile(), 0,
                                                exheader_hash_region_size),
                  std::make_shared<FB::SubFile>(this->file, 0x160, 0x20));
            }},
           {"Signature", [this]() {
              return std::make_shared<Rsa>(
                  std::make_shared<FB::SubFile>(this->file, 0x100, 0x100),
                  std::make_shared<FB::SubFile>(this->file, 0, 0x100),
                  Open("Exheader")
                      ->Open("NcchSignaturePublicKey")
                      ->ValueT<FB::FilePtr>());
            }}});
    } else {
      InstallList(
          {{"Signature", [this]() {
              return std::make_shared<Rsa>(
                  std::make_shared<FB::SubFile>(this->file, 0x100, 0x100),
                  std::make_shared<FB::SubFile>(this->file, 0, 0x100),
                  std::make_shared<FB::MemoryFile>());
            }}});
    }
  } else {
    auto signature_key = secrets[SB::k_sec_pubkey_ncsd_cfa];
    InstallList({
        {"Signature",
         [this, signature_key]() {
           return std::make_shared<Rsa>(
               std::make_shared<FB::SubFile>(this->file, 0x100, 0x100),
               std::make_shared<FB::SubFile>(this->file, 0, 0x100),
               std::make_shared<FB::MemoryFile>(signature_key));
         }},
    });
  }

  u32 exefs_size = Open("ExefsOffset")->ValueT<u32>();
  if (exefs_size) {
    auto error = ExefsError();
    InstallList({{"ExefsError", [error]() {
                    return std::make_shared<ConstContainer>(error);
                  }}});
    if (error.empty()) {
      InstallList({
          {"Exefs",
           [this]() {
             return std::make_shared<Exefs>(PrimaryExefsFile(),
                                            SecondaryExefsFile());
           }},
          {"ExefsHash",
           [this]() {
             u32 region_size = Open("ExefsHashRegionSize")->ValueT<u32>();
             return std::make_shared<Sha>(
                 std::make_shared<FB::SubFile>(PrimaryExefsFile(), 0,
                                               region_size * 0x200),
                 std::make_shared<FB::SubFile>(this->file, 0x1C0, 0x20));
           }},
      });
    }
  }

  u32 romfs_size = Open("RomfsOffset")->ValueT<u32>();
  if (romfs_size) {
    auto error = RomfsError();
    InstallList({{"RomfsError", [error]() {
                    return std::make_shared<ConstContainer>(error);
                  }}});
    if (error.empty()) {
      InstallList({
          {"Romfs", [this]() { return std::make_shared<Romfs>(RomfsFile()); }},
          {"RomfsHash",
           [this]() {
             u32 region_size = Open("RomfsHashRegionSize")->ValueT<u32>();
             return std::make_shared<Sha>(
                 std::make_shared<FB::SubFile>(RomfsFile(), 0,
                                               region_size * 0x200),
                 std::make_shared<FB::SubFile>(this->file, 0x1E0, 0x20));
           }},
      });
    }
  }
}

void Ncch::CheckForceNoCrypto() {
  if (Open("IsNoCrypto")->ValueT<bool>())
    return;

  if (!Open("RomfsOffset")->ValueT<u32>())
    return;

  if (RawRomfsFile()->Read<magic_t>(0) == magic_t{'I', 'V', 'F', 'C'}) {
    force_no_crypto = true;
  }
}

FB::FilePtr Ncch::KeyY() {
  return std::make_shared<FB::SubFile>(file, 0, 0x10);
  // TODO Seed crypto
}

FB::FilePtr Ncch::PrimaryNormalKey() {
  if (Open("IsFixedKeyCrypto")->ValueT<bool>()) {
    // TODO: system fixed key
    return std::make_shared<FB::MemoryFile>(0x10, byte{0});
  }
  auto key_y_buf = KeyY()->Read(0, 0x10);
  AESKey key_x, key_y, key_c;
  std::memcpy(key_x.data(), secrets[SB::k_sec_key2C_x].data(), 0x10);
  std::memcpy(key_y.data(), key_y_buf.data(), 0x10);
  std::memcpy(key_c.data(), secrets[SB::k_sec_aes_const].data(), 0x10);
  AESKey normal = ScrambleKey(key_x, key_y, key_c);
  return std::make_shared<FB::MemoryFile>(normal.begin(), normal.end());
}

FB::FilePtr Ncch::SecondaryNormalKey() {
  if (Open("IsFixedKeyCrypto")->ValueT<bool>()) {
    // TODO: system fixed key
    return std::make_shared<FB::MemoryFile>(0x10, byte{0});
  }
  auto key_y_buf = KeyY()->Read(0, 0x10);
  AESKey key_x, key_y, key_c;
  std::memcpy(key_y.data(), key_y_buf.data(), 0x10);
  std::memcpy(key_c.data(), secrets[SB::k_sec_aes_const].data(), 0x10);
  switch (Open("CryptoMethod")->ValueT<u8>()) {
  case 0x00:
    std::memcpy(key_x.data(), secrets[SB::k_sec_key2C_x].data(), 0x10);
    break;
  case 0x01:
    std::memcpy(key_x.data(), secrets[SB::k_sec_key25_x].data(), 0x10);
    break;
  case 0x0A:
    std::memcpy(key_x.data(), secrets[SB::k_sec_key18_x].data(), 0x10);
    break;
  case 0x0B:
    std::memcpy(key_x.data(), secrets[SB::k_sec_key1B_x].data(), 0x10);
    break;
  default:
    throw;
  }

  std::memcpy(key_y.data(), key_y_buf.data(), 0x10);
  std::memcpy(key_c.data(), secrets[SB::k_sec_aes_const].data(), 0x10);
  AESKey normal = ScrambleKey(key_x, key_y, key_c);
  return std::make_shared<FB::MemoryFile>(normal.begin(), normal.end());
}

std::string Ncch::PrimaryNormalKeyError() {
  if (Open("IsFixedKeyCrypto")->ValueT<bool>()) {
    // TODO: system fixed key
    return "";
  }
  if (secrets[SB::k_sec_aes_const].size() != 16)
    return SB::k_sec_aes_const;
  if (secrets[SB::k_sec_key2C_x].size() != 16)
    return SB::k_sec_key2C_x;
  return "";
}

std::string Ncch::SecondaryNormalKeyError() {
  if (Open("IsFixedKeyCrypto")->ValueT<bool>()) {
    // TODO: system fixed key
    return "";
  }
  if (secrets[SB::k_sec_aes_const].size() != 16)
    return SB::k_sec_aes_const;
  switch (Open("CryptoMethod")->ValueT<u8>()) {
  case 0x00:
    if (secrets[SB::k_sec_key2C_x].size() != 16)
      return SB::k_sec_key2C_x;
    break;
  case 0x01:
    if (secrets[SB::k_sec_key25_x].size() != 16)
      return SB::k_sec_key25_x;
    break;
  case 0x0A:
    if (secrets[SB::k_sec_key18_x].size() != 16)
      return SB::k_sec_key18_x;
    break;
  case 0x0B:
    if (secrets[SB::k_sec_key1B_x].size() != 16)
      return SB::k_sec_key1B_x;
    break;
  default:
    return "???";
  }
  return "";
}

bool Ncch::IsDecrypted() {
  return force_no_crypto || Open("IsNoCrypto")->ValueT<bool>();
}

FB::FilePtr Ncch::CryptoIv(IvType type) {
  u16 version = Open("Version")->ValueT<u16>();
  if (version == 1) {
    throw;
  }

  u64 partition_id = Open("PartitionId")->ValueT<u64>();
  std::array<byte, 8> partition_id_s;
  std::memcpy(partition_id_s.data(), &partition_id, 8);
  auto iv = std::make_shared<FB::MemoryFile>(16);
  std::reverse_copy(partition_id_s.begin(), partition_id_s.end(), iv->begin());
  (*iv)[8] = byte{type};
  return iv;
}

FB::FilePtr Ncch::ExheaderFile() {
  auto raw = std::make_shared<FB::SubFile>(file, 0x200, 0x800);
  if (IsDecrypted()) {
    return raw;
  }

  auto iv = CryptoIv(IvType::Exheader);
  return std::make_shared<FB::AesCtrFile>(raw, PrimaryNormalKey(), iv);
}

std::string Ncch::ExheaderError() {
  if (IsDecrypted()) {
    return "";
  }
  return PrimaryNormalKeyError();
}

FB::FilePtr Ncch::RawExefsFile() {
  return std::make_shared<FB::SubFile>(
      file, Open("ExefsOffset")->ValueT<u32>() * 0x200,
      Open("ExefsSize")->ValueT<u32>() * 0x200);
}

FB::FilePtr Ncch::PrimaryExefsFile() {
  auto raw = RawExefsFile();
  if (IsDecrypted()) {
    return raw;
  }

  auto iv = CryptoIv(IvType::Exefs);
  return std::make_shared<FB::AesCtrFile>(raw, PrimaryNormalKey(), iv);
}

FB::FilePtr Ncch::SecondaryExefsFile() {
  auto raw = RawExefsFile();
  if (IsDecrypted()) {
    return raw;
  }

  auto iv = CryptoIv(IvType::Exefs);
  return std::make_shared<FB::AesCtrFile>(raw, SecondaryNormalKey(), iv);
}

std::string Ncch::ExefsError() {
  if (IsDecrypted()) {
    return "";
  }

  auto error_primary = PrimaryNormalKeyError();
  if (!error_primary.empty())
    return error_primary;
  return SecondaryNormalKeyError();
}

FB::FilePtr Ncch::RawRomfsFile() {
  return std::make_shared<FB::SubFile>(
      file, Open("RomfsOffset")->ValueT<u32>() * 0x200,
      Open("RomfsSize")->ValueT<u32>() * 0x200);
}

FB::FilePtr Ncch::RomfsFile() {
  auto raw = RawRomfsFile();

  if (IsDecrypted()) {
    return raw;
  }

  auto iv = CryptoIv(IvType::Romfs);
  return std::make_shared<FB::AesCtrFile>(raw, SecondaryNormalKey(), iv);
}

std::string Ncch::RomfsError() {
  if (IsDecrypted()) {
    return "";
  }
  return SecondaryNormalKeyError();
}

u8 Ncch::ContentType() { return Open("ContentTypeFlags")->ValueT<u8>(); }

u8 Ncch::ContentType2() { return Open("ContentType2")->ValueT<u8>(); }

} // namespace CB