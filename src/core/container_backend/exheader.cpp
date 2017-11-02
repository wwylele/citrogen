#include "core/container_backend/exheader.h"
#include "core/container_backend/rsa.h"
#include "core/file_backend/memory_file.h"

namespace CB {

Exheader::Exheader(FB::FilePtr file) : FileContainer(std::move(file)) {
  auto signature_key = secrets[SB::k_sec_pubkey_exheader];
  InstallList({
      {"Signature",
       [this, signature_key]() {
         return std::make_shared<Rsa>(
             std::make_shared<FB::SubFile>(this->file, 0x500, 0x300),
             std::make_shared<FB::SubFile>(this->file, 0x400, 0x100),
             std::make_shared<FB::MemoryFile>(signature_key));
       }},
      {"NcchSignaturePublicKey",
       [this]() {
         return std::make_shared<FileContainer>(
             std::make_shared<FB::SubFile>(this->file, 0x500, 0x100));
       }},

      Field<std::array<char, 8>>("Name", 0x0),

      {"IsCodeCompressed",
       [this]() {
         return std::make_shared<ConstContainer>((SciFlags() & 1) != 0);
       }},
      {"IsSdApp",
       [this]() {
         return std::make_shared<ConstContainer>((SciFlags() & 2) != 0);
       }},
      Field<u16>("RemasterVersion", 0xE),
  });
}

u8 Exheader::SciFlags() { return file->Read<u8>(0xD); }

} // namespace CB