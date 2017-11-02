#include "core/container_backend/cia.h"
#include "core/aes_key.h"
#include "core/align.h"
#include "core/container_backend/ncch.h"
#include "core/container_backend/sha.h"
#include "core/file_backend/aes_cbc.h"
#include "core/file_backend/memory_file.h"
#include "core/secret_backend/secret_database.h"
#include <unordered_map>

namespace CB {

std::unordered_map<u32, std::size_t> signature_size{
    {0x10003, 0x240},
    {0x10004, 0x140},
    {0x10005, 0x80},
};

class Ticket : public FileContainer {
public:
  Ticket(FB::FilePtr file_) : FileContainer(std::move(file_)) {
    InstallList({
        Field<be_<u32>>("SignatureType", 0x0),
    });
    main_offset = signature_size[Open("SignatureType")->ValueT<u32>()];

    InstallList({
        Field<u8>("KeyIndex", 0xB1 + main_offset),
    });

    std::string error = TitleKeyError();
    InstallList({
        {"TitleKeyError",
         [error]() { return std::make_shared<ConstContainer>(error); }},
    });
    if (error.empty())
      InstallList({
          {"TitleKey",
           [this]() { return std::make_shared<FileContainer>(TitleKey()); }},
      });
  }

private:
  std::size_t main_offset;
  SB::SecretContext secrets;

  std::string TitleKeyError() {
    if (secrets[SB::k_sec_aes_const].size() != 0x10)
      return SB::k_sec_aes_const;
    if (secrets[SB::k_sec_key3D_x].size() != 0x10)
      return SB::k_sec_key3D_x;
    u8 key_index = Open("KeyIndex")->ValueT<u8>();
    if (secrets[SB::k_sec_key3D_y[key_index]].size() != 0x10)
      return SB::k_sec_key3D_y[key_index];
    return "";
  }

  FB::FilePtr TitleKey() {
    auto encrypted =
        std::make_shared<FB::SubFile>(file, 0x7F + main_offset, 0x10);
    byte_seq title_id; // gcc 7 bug: must separate here
    title_id = file->Read(0x9C + main_offset, 8);
    auto iv =
        std::make_shared<FB::MemoryFile>(title_id.begin(), title_id.end());
    iv->resize(16, byte{0});
    u8 key_index = Open("KeyIndex")->ValueT<u8>();
    AESKey x, y, c;
    std::memcpy(x.data(), secrets[SB::k_sec_key3D_x].data(), 0x10);
    std::memcpy(y.data(), secrets[SB::k_sec_key3D_y[key_index]].data(), 0x10);
    std::memcpy(c.data(), secrets[SB::k_sec_aes_const].data(), 0x10);
    auto key_data = ScrambleKey(x, y, c);
    auto key =
        std::make_shared<FB::MemoryFile>(key_data.begin(), key_data.end());
    return std::make_shared<FB::AesCbcFile>(encrypted, key, iv);
  }
};

class Tmd : public FileContainer {
public:
  Tmd(FB::FilePtr file_) : FileContainer(std::move(file_)) {
    InstallList({
        Field<be_<u32>>("SignatureType", 0x0),
    });
    main_offset = signature_size[Open("SignatureType")->ValueT<u32>()];

    InstallList({
        Field<be_<u16>>("ContentCount", 0x9E + main_offset),
    });

    u16 content_count = Open("ContentCount")->ValueT<u16>();
    for (u16 i = 0; i < content_count; ++i) {
      InstallList({
          Field<be_<u32>>(WithIndex("ContentId", i),
                          0x9C4 + main_offset + i * 0x30 + 0x0),
          Field<be_<u16>>(WithIndex("ContentIndex", i),
                          0x9C4 + main_offset + i * 0x30 + 0x4),
          Field<be_<u16>>(WithIndex("ContentType", i),
                          0x9C4 + main_offset + i * 0x30 + 0x6),
          Field<be_<u64>>(WithIndex("ContentSize", i),
                          0x9C4 + main_offset + i * 0x30 + 0x8),
          {WithIndex("ContentHash", i),
           [this, i]() {
             return std::make_shared<FileContainer>(
                 std::make_shared<FB::SubFile>(
                     file, 0x9C4 + main_offset + i * 0x30 + 0x10, 0x20));
           }},

          {WithIndex("ContentIv", i),
           [this, i]() {
             byte_seq iv_data; // gcc 7 bug: must separate here
             iv_data = file->Read(0x9C4 + main_offset + i * 0x30 + 0x4, 2);
             iv_data.resize(16, byte{0});
             auto iv = std::make_shared<FB::MemoryFile>(iv_data.begin(),
                                                        iv_data.end());
             return std::make_shared<FileContainer>(iv);
           }},
          {WithIndex("ContentIsEncrypted", i),
           [this, i]() {
             return std::make_shared<ConstContainer>(
                 (Open(WithIndex("ContentType", i))->ValueT<u16>() & 0x1) != 0);
           }},
          {WithIndex("ContentIsDisc", i),
           [this, i]() {
             return std::make_shared<ConstContainer>(
                 (Open(WithIndex("ContentType", i))->ValueT<u16>() & 0x2) != 0);
           }},
          {WithIndex("ContentIsCfm", i),
           [this, i]() {
             return std::make_shared<ConstContainer>(
                 (Open(WithIndex("ContentType", i))->ValueT<u16>() & 0x4) != 0);
           }},
          {WithIndex("ContentIsOptional", i),
           [this, i]() {
             return std::make_shared<ConstContainer>(
                 (Open(WithIndex("ContentType", i))->ValueT<u16>() & 0x4000) !=
                 0);
           }},
          {WithIndex("ContentIsShared", i),
           [this, i]() {
             return std::make_shared<ConstContainer>(
                 (Open(WithIndex("ContentType", i))->ValueT<u16>() & 0x8000) !=
                 0);
           }},
      });
    }
  }

private:
  std::size_t main_offset;
};

Cia::Cia(FB::FilePtr file_) : FileContainer(std::move(file_)) {
  InstallList({
      Field<u32>("HeaderSize", 0x0),
      Field<u16>("Type", 0x4),
      Field<u16>("Version", 0x6),
      Field<u32>("CertificateChainSize", 0x8),
      Field<u32>("TicketSize", 0xC),
      Field<u32>("TmdSize", 0x10),
      Field<u32>("MetadataSize", 0x14),
      Field<u64>("ContentSize", 0x18),
  });

  u64 offset = 0;
  offset += Open("HeaderSize")->ValueT<u32>();
  offset = AlignUp(offset, 64);

  u32 certificate_chain_size = Open("CertificateChainSize")->ValueT<u32>();
  // FB::FilePtr certificate_chain = std::make_shared<FB::SubFile>(file, offset,
  // certificate_chain_size);
  offset += certificate_chain_size;
  offset = AlignUp(offset, 64);

  u32 ticket_size = Open("TicketSize")->ValueT<u32>();
  FB::FilePtr ticket = std::make_shared<FB::SubFile>(file, offset, ticket_size);
  offset += ticket_size;
  offset = AlignUp(offset, 64);

  u32 tmd_size = Open("TmdSize")->ValueT<u32>();
  FB::FilePtr tmd = std::make_shared<FB::SubFile>(file, offset, tmd_size);
  offset += tmd_size;
  offset = AlignUp(offset, 64);

  u64 content_size = Open("ContentSize")->ValueT<u64>();
  content = std::make_shared<FB::SubFile>(file, offset, content_size);
  offset += content_size;
  offset = AlignUp(offset, 64);

  u32 metadata_size = Open("MetadataSize")->ValueT<u32>();
  metadata = std::make_shared<FB::SubFile>(file, offset, metadata_size);

  InstallList({
      {"Ticket", [ticket]() { return std::make_shared<Ticket>(ticket); }},
      {"Tmd", [tmd]() { return std::make_shared<Tmd>(tmd); }},
  });

  auto tmd_container = Open("Tmd");
  auto ticket_container = Open("Ticket");
  std::string title_key_error =
      ticket_container->Open("TitleKeyError")->ValueT<std::string>();
  FB::FilePtr title_key;
  if (title_key_error.empty()) {
    title_key = ticket_container->Open("TitleKey")->ValueT<FB::FilePtr>();
  }
  u16 content_count = tmd_container->Open("ContentCount")->ValueT<u16>();
  std::size_t content_offset = 0;
  for (u16 i = 0; i < content_count; ++i) {
    u64 size = tmd_container->Open(WithIndex("ContentSize", i))->ValueT<u64>();
    auto raw = std::make_shared<FB::SubFile>(content, content_offset, size);
    content_offset += size;

    if (tmd_container->Open(WithIndex("ContentIsEncrypted", i))
            ->ValueT<bool>()) {
      InstallList({{WithIndex("ContentError", i), [title_key_error]() {
                      return std::make_shared<ConstContainer>(title_key_error);
                    }}});
      if (title_key_error.empty()) {
        auto iv = tmd_container->Open(WithIndex("ContentIv", i))
                      ->ValueT<FB::FilePtr>();
        sub_contents.push_back(
            std::make_shared<FB::AesCbcFile>(raw, title_key, iv));
      } else {
        sub_contents.push_back(nullptr);
      }
    } else {
      InstallList({{WithIndex("ContentError", i), [title_key_error]() {
                      return std::make_shared<ConstContainer>(std::string());
                    }}});
      sub_contents.push_back(raw);
    }

    if (sub_contents[i]) {
      InstallList({
          {WithIndex("Content", i),
           [this, i]() { return std::make_shared<Ncch>(sub_contents[i]); }},
          {WithIndex("ContentHash", i),
           [this, i]() {
             auto hash = Open("Tmd")
                             ->Open(WithIndex("ContentHash", i))
                             ->ValueT<FB::FilePtr>();
             return std::make_shared<Sha>(sub_contents[i], hash);
           }},
      });
    }
  }
}

} // namespace CB
