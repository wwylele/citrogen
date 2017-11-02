#include "sha.h"
#include "core/cryptopp_util.h"
#include <cryptopp/sha.h>

namespace CB {

Sha::Sha(FB::FilePtr data, FB::FilePtr hash)
    : data(std::move(data)), hash(std::move(hash)) {
  InstallList({
      {"Match",
       [this]() {
         byte_seq hash(CryptoPP::SHA256::DIGESTSIZE);
         std::size_t size = this->data->GetSize();
         auto buffer = this->data->Read(0, size);
         CryptoPP::SHA256().CalculateDigest(CryptoPPBytes(hash),
                                            CryptoPPBytes(buffer), size);
         auto hash2 = this->hash->Read(0, CryptoPP::SHA256::DIGESTSIZE);
         return std::make_shared<ConstContainer>(hash == hash2);
       }},
  });
}

std::any Sha::Value() { return hash->Read(0, CryptoPP::SHA256::DIGESTSIZE); }

} // namespace CB