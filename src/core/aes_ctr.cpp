#include "core/aes_ctr.h"
#include <crypto++/aes.h>
#include <crypto++/modes.h>
#include <cstdio>
namespace FB {

AesCtrFile::AesCtrFile(FilePtr parent, FilePtr key, FilePtr iv)
    : parent(std::move(parent)), key(std::move(key)), iv(std::move(iv)) {}

std::size_t AesCtrFile::GetSize() { return parent->GetSize(); }

std::vector<u8> AesCtrFile::Read(std::size_t pos, std::size_t size) {
  auto buffer = parent->Read(pos, size);
  auto key_data = key->Read(0, 16);
  auto iv_data = iv->Read(0, 16);
  if (key_data.size() != 16 || iv_data.size() != 16)
    return {};
  CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption dec;
  dec.SetKeyWithIV(key_data.data(), 16, iv_data.data(), 16);
  dec.Seek(pos);
  dec.ProcessData(buffer.data(), buffer.data(), buffer.size());
  return buffer;
}
} // namespace FB