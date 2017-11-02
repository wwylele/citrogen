#include "core/file_backend/aes_ctr.h"
#include "core/cryptopp_util.h"
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cstdio>
namespace FB {

AesCtrFile::AesCtrFile(FilePtr parent, FilePtr key, FilePtr iv)
    : parent(std::move(parent)), key(std::move(key)), iv(std::move(iv)) {}

std::size_t AesCtrFile::GetSize() { return parent->GetSize(); }

byte_seq AesCtrFile::Read(std::size_t pos, std::size_t size) {
  auto buffer = parent->Read(pos, size);
  auto key_data = key->Read(0, 16);
  auto iv_data = iv->Read(0, 16);
  if (key_data.size() != 16 || iv_data.size() != 16)
    return {};
  CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption dec;
  dec.SetKeyWithIV(CryptoPPBytes(key_data), 16, CryptoPPBytes(iv_data), 16);
  dec.Seek(pos);
  dec.ProcessData(CryptoPPBytes(buffer), CryptoPPBytes(buffer), buffer.size());
  return buffer;
}
} // namespace FB