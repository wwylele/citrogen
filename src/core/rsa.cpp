#include "core/rsa.h"
#include "cryptopp_util.h"
#include <cryptopp/rsa.h>

namespace CB {

Rsa::Rsa(FB::FilePtr data_, FB::FilePtr signature_, FB::FilePtr public_key_)
    : data(std::move(data_)), signature(std::move(signature_)),
      public_key(std::move(public_key_)) {
  if (public_key->GetSize() == 0x100) {
    InstallList({
        {"Match",
         [this]() {
           std::size_t data_len = this->data->GetSize();
           auto d = this->data->Read(0, data_len);
           auto s = this->signature->Read(0, 0x100);
           auto n = this->public_key->Read(0, 0x100);
           if (s.size() != 0x100 || n.size() != 0x100) {
             return std::make_shared<ConstContainer>(false);
           }
           bool result = false;
           try {
             result = CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA256>::
                          Verifier(CryptoPP::Integer(CryptoPPBytes(n), 0x100),
                                   CryptoPP::Integer(0x10001))
                              .VerifyMessage(CryptoPPBytes(d), data_len,
                                             CryptoPPBytes(s), 0x100);
           } catch (...) {
           }
           return std::make_shared<ConstContainer>(result);
         }},
    });
  }
}

std::any Rsa::Value() { return signature->Read(0, 0x100); }

} // namespace CB