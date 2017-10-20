#include "core/sha.h"
#include <cryptopp/sha.h>

namespace CB {

Sha::Sha(FB::FilePtr data, FB::FilePtr hash)
    : data(std::move(data)), hash(std::move(hash)) {
  InstallList({
      {"Match",
       [this]() {
         std::array<u8, CryptoPP::SHA256::DIGESTSIZE> hash;
         std::size_t size = this->data->GetSize();
         auto buffer = this->data->Read(0, size);
         CryptoPP::SHA256().CalculateDigest(hash.data(), buffer.data(), size);
         auto hash2 = this->hash->Read(0, CryptoPP::SHA256::DIGESTSIZE);
         return std::make_shared<ConstContainer>(
             std::memcmp(hash.data(), hash2.data(),
                         CryptoPP::SHA256::DIGESTSIZE) == 0);
       }},
  });
}

std::any Sha::Value() { return hash->Read(0, CryptoPP::SHA256::DIGESTSIZE); }

} // namespace CB