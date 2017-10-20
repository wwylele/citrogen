#include "core/aes_cbc.h"
#include "core/align.h"
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>

namespace FB {

AesCbcFile::AesCbcFile(FilePtr parent, FilePtr key, FilePtr iv)
    : parent(std::move(parent)), key(std::move(key)), iv(std::move(iv)) {}

std::size_t AesCbcFile::GetSize() { return parent->GetSize(); }

std::vector<u8> AesCbcFile::Read(std::size_t pos, std::size_t size) {
  // Random access for CBC mode decryption is implemented here.

  // read encrypted data from aligned boundary
  std::size_t end = pos + size;
  std::size_t pos_align_down = AlignDown(pos, 16);
  std::size_t end_align_up = std::min(AlignUp(end, 16), GetSize());
  std::size_t size_align = end_align_up - pos_align_down;
  auto buffer = parent->Read(pos_align_down, size_align);

  // if the boundary doesn't start from the file beginning, use the previous
  // encrypted block as iv
  auto iv_data = pos_align_down == 0 ? iv->Read(0, 16)
                                     : parent->Read(pos_align_down - 16, 16);

  // decrypt
  auto key_data = key->Read(0, 16);
  if (key_data.size() != 16 || iv_data.size() != 16)
    return {};
  CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption dec;
  dec.SetKeyWithIV(key_data.data(), 16, iv_data.data(), 16);
  dec.ProcessData(buffer.data(), buffer.data(), buffer.size());

  // truncate the buffer to the actual boundary
  return std::vector<u8>(buffer.begin() + (pos - pos_align_down),
                         buffer.end() - (end_align_up - end));
}
} // namespace FB