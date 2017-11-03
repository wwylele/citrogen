#include "core/container_backend/sd_protected.h"
#include "core/aes_key.h"
#include "core/cryptopp_util.h"
#include "core/file_backend/aes_ctr.h"
#include "core/file_backend/memory_file.h"
#include "core/string_util.h"
#include <cryptopp/sha.h>

namespace CB {

class SdProtectedEntry : public ContainerHelper {
public:
  SdProtectedEntry(ContainerPtr base_, const std::string &path_,
                   const AESKey &key_)
      : base(std::move(base_)), path(path_), key(key_) {
    auto list = base->List();
    for (const std::string &name : list) {
      InstallList({{name, [this, name]() {
                      return std::make_shared<SdProtectedEntry>(
                          base->Open(name), path + "/" + name, key);
                    }}});
    }
  }

  std::any Value() override {
    auto base_file_any = base->Value();
    if (base_file_any.type() != typeid(FB::FilePtr))
      return {};
    auto base_file = std::any_cast<FB::FilePtr>(base_file_any);

    std::u16string path_to_hash;
    // TODO proper UTF-8 to UTF-16
    for (char c : path)
      path_to_hash += (char16_t)c;
    byte_seq hash(CryptoPP::SHA256::DIGESTSIZE);
    CryptoPP::SHA256().CalculateDigest(
        CryptoPPBytes(hash),
        reinterpret_cast<const CryptoPP::byte *>(path_to_hash.c_str()),
        (path_to_hash.size() + 1) * 2);

    AESKey iv;
    for (unsigned i = 0; i < 16; i++) {
      iv[i] = hash[i] ^ hash[i + 16];
    }

    FB::FilePtr key_file =
        std::make_shared<FB::MemoryFile>(key.begin(), key.end());
    FB::FilePtr iv_file =
        std::make_shared<FB::MemoryFile>(iv.begin(), iv.end());

    return std::static_pointer_cast<FB::File>(
        std::make_shared<FB::AesCtrFile>(base_file, key_file, iv_file));
  }

private:
  ContainerPtr base;
  std::string path;
  AESKey key;
};

SdProtected::SdProtected(ContainerPtr sd_root) {
  // TODO exceptions
  auto key_x = secrets[SB::k_sec_key34_x];
  auto key_y = secrets[SB::k_sec_key34_y];
  auto key_c = secrets[SB::k_sec_aes_const];
  if (key_x.size() != 0x10 || key_y.size() != 0x10 || key_c.size() != 0x10) {
    return;
  }

  AESKey x, y, c;
  std::memcpy(x.data(), key_x.data(), 0x10);
  std::memcpy(y.data(), key_y.data(), 0x10);
  std::memcpy(c.data(), key_c.data(), 0x10);
  AESKey key = ScrambleKey(x, y, c);

  auto nin_root = sd_root->Open("Nintendo 3DS");
  if (!nin_root) {
    return;
  }

  byte_seq hash(CryptoPP::SHA256::DIGESTSIZE);
  CryptoPP::SHA256().CalculateDigest(CryptoPPBytes(hash), CryptoPPBytes(key_y),
                                     0x10);

  std::string id0;
  for (unsigned index :
       {3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12}) {
    id0 += IntToHex<false>((u8)hash[index]);
  }

  nin_root = nin_root->Open(id0);
  if (!nin_root) {
    return;
  }

  auto id1 = nin_root->List();
  if (id1.size() != 1) {
    return;
  }

  nin_root = nin_root->Open(id1[0]);
  InstallList({{"Root", [nin_root, key]() {
                  return std::make_shared<SdProtectedEntry>(nin_root, "", key);
                }}});
}

} // namespace CB