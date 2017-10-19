#include "core/ncsd.h"
#include "core/ncch.h"
#include "core/rsa.h"
#include "memory_file.h"

namespace CB {

Ncsd::Ncsd(FB::FilePtr file) : FileContainer(std::move(file)) {
  auto signature_key = secrets[SB::k_sec_pubkey_ncsd_cfa];
  InstallList({
      {"Signature",
       [this, signature_key]() {
         return std::make_shared<Rsa>(
             std::make_shared<FB::SubFile>(this->file, 0x100, 0x100),
             std::make_shared<FB::SubFile>(this->file, 0, 0x100),
             std::make_shared<FB::MemoryFile>(signature_key));
       }},

      Field<magic_t>("Magic", 0x100),
      Field<u32>("ImageSize", 0x104),
      Field<u64>("MediaId", 0x108),
      // skip some
  });

  for (std::size_t i = 0; i < 8; ++i) {
    InstallList({
        Field<u32>(WithIndex("PartitionOffset", i), 0x120 + i * 8),
        Field<u32>(WithIndex("PartitionSize", i), 0x124 + i * 8),
    });
    u32 offset = Open(WithIndex("PartitionOffset", i))->ValueT<u32>();
    u32 size = Open(WithIndex("PartitionSize", i))->ValueT<u32>();
    if (offset && size) {
      auto partition_opener = [this, offset, size]() {
        return std::make_shared<Ncch>(std::make_shared<FB::SubFile>(
            this->file, offset * 0x200, size * 0x200));

      };
      InstallList({
          {WithIndex("Partition", i), partition_opener},
      });
    }
  }
}

} // namespace CB