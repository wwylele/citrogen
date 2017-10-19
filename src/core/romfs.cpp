#include "core/romfs.h"
#include "core/align.h"
#include "core/sha.h"

namespace CB {

class Level3Cursor : public ContainerHelper {
  struct DirectoryEntry {
    u32 parent;
    u32 sibling_directory;
    u32 child_directory;
    u32 child_file;
    u32 next_collision;
  };
  struct FileEntry {
    u32 parent;
    u32 sibling_file;
    u64 data_offset;
    u64 data_size;
    u32 next_collision;
  };

  static std::string ReadName(FB::FilePtr file, u32 offset) {
    u32 length = file->Read<u32>(offset);
    auto str = file->Read(offset + 4, length);
    std::string result;
    // HACK
    for (auto iter = str.begin(); iter != str.end(); iter += 2) {
      result += (char)*iter;
    }
    return result;
  }

public:
  Level3Cursor(FB::FilePtr directory_metadata_, FB::FilePtr file_metadata_,
               FB::FilePtr file_data_, u32 directory_offset)
      : directory_metadata(std::move(directory_metadata_)),
        file_metadata(std::move(file_metadata_)),
        file_data(std::move(file_data_)) {
    auto self = directory_metadata->Read<DirectoryEntry>(directory_offset);

    u32 child_offset = self.child_directory;
    while (child_offset != 0xFFFFFFFF) {
      auto child = directory_metadata->Read<DirectoryEntry>(child_offset);
      std::string name = ReadName(directory_metadata, child_offset + 0x14);
      InstallList({{name, [this, child_offset]() {
                      return std::make_shared<Level3Cursor>(
                          directory_metadata, file_metadata, file_data,
                          child_offset);
                    }}});
      child_offset = child.sibling_directory;
    }

    child_offset = self.child_file;
    while (child_offset != 0xFFFFFFFF) {
      auto child = file_metadata->Read<FileEntry>(child_offset);
      std::string name = ReadName(file_metadata, child_offset + 0x1C);
      u64 offset = child.data_offset;
      u64 size = child.data_size;
      InstallList(
          {{name, [this, offset, size]() {
              return std::make_shared<FileContainer>(
                  std::make_shared<FB::SubFile>(file_data, offset, size));
            }}});
      child_offset = child.sibling_file;
    }
  }

private:
  FB::FilePtr directory_metadata;
  FB::FilePtr file_metadata;
  FB::FilePtr file_data;
};

class Level3 : public FileContainer {
public:
  Level3(FB::FilePtr file_) : FileContainer(std::move(file_)) {
    InstallList({Field<u32>("DirectoryHashTableOffset", 0x4),
                 Field<u32>("DirectoryHashTableSize", 0x8),
                 Field<u32>("DirectoryMetadataOffset", 0xC),
                 Field<u32>("DirectoryMetadataSize", 0x10),
                 Field<u32>("FileHashTableOffset", 0x14),
                 Field<u32>("FileHashTableSize", 0x18),
                 Field<u32>("FileMetadataOffset", 0x1C),
                 Field<u32>("FileMetadataSize", 0x20),
                 Field<u32>("FileDataOffset", 0x24),
                 {".", [this]() {
                    return std::make_shared<Level3Cursor>(
                        DirectoryMetadata(), FileMetadata(), FileData(), 0);
                  }}});
  }

private:
  FB::FilePtr DirectoryHashTable() {
    return std::make_shared<FB::SubFile>(
        file, Open("DirectoryHashTableOffset")->ValueT<u32>(),
        Open("DirectoryHashTableSize")->ValueT<u32>());
  }
  FB::FilePtr DirectoryMetadata() {
    return std::make_shared<FB::SubFile>(
        file, Open("DirectoryMetadataOffset")->ValueT<u32>(),
        Open("DirectoryMetadataSize")->ValueT<u32>());
  }
  FB::FilePtr FileHashTable() {
    return std::make_shared<FB::SubFile>(
        file, Open("FileHashTableOffset")->ValueT<u32>(),
        Open("FileHashTableSize")->ValueT<u32>());
  }
  FB::FilePtr FileMetadata() {
    return std::make_shared<FB::SubFile>(
        file, Open("FileMetadataOffset")->ValueT<u32>(),
        Open("FileMetadataSize")->ValueT<u32>());
  }

  FB::FilePtr FileData() {
    u32 offset = Open("FileDataOffset")->ValueT<u32>();
    std::size_t size = file->GetSize() - offset;
    return std::make_shared<FB::SubFile>(file, offset, size);
  }
};

class ShaList : public ContainerHelper {
public:
  ShaList(FB::FilePtr data_, FB::FilePtr hash_, std::size_t block_size)
      : data(std::move(data_)), hash(std::move(hash_)) {
    u64 size = hash->GetSize() / 0x20;
    if (size != data->GetSize() / block_size) {
      printf("%08X %08X \n", (int)size, (int)(data->GetSize() / block_size));
    }
    InstallList({
        {"Size", [size]() { return std::make_shared<ConstContainer>(size); }},
    });
    for (u64 i = 0; i < size; ++i) {
      InstallList({{WithIndex("Hash", i), [i, block_size, this]() {
                      return std::make_shared<Sha>(
                          std::make_shared<FB::SubFile>(data, i * block_size,
                                                        block_size),
                          std::make_shared<FB::SubFile>(hash, i * 0x20, 0x20));
                    }}});
    }

    InstallList(
        {{"Match", [this, size]() {
            for (u64 i = 0; i < size; ++i) {
              printf("%lu\n", i);
              if (!Open(WithIndex("Hash", i))->Open("Match")->ValueT<bool>()) {
                return std::make_shared<ConstContainer>(false);
              }
            }
            return std::make_shared<ConstContainer>(true);
          }}});
  }

private:
  FB::FilePtr data;
  FB::FilePtr hash;
};

Romfs::Romfs(FB::FilePtr file_) : FileContainer(std::move(file_)) {
  InstallList({
      Field<u32>("Level0Size", 0x08),

      Field<u64>("Level1Offset", 0x0C),
      Field<u64>("Level1Size", 0x14),
      {"Level1BlockSize",
       [this]() {
         return std::make_shared<ConstContainer>(
             u64(1 << file->Read<u32>(0x1C)));
       }},

      Field<u64>("Level2Offset", 0x24),
      Field<u64>("Level2Size", 0x2C),
      {"Level2BlockSize",
       [this]() {
         return std::make_shared<ConstContainer>(
             u64(1 << file->Read<u32>(0x34)));
       }},

      Field<u64>("Level3Offset", 0x3C),
      Field<u64>("Level3Size", 0x44),
      {"Level3BlockSize",
       [this]() {
         return std::make_shared<ConstContainer>(
             u64(1 << file->Read<u32>(0x4C)));
       }},

      {"Level0",
       [this]() {
         return std::make_shared<ShaList>(
             Level1File(true), Level0File(),
             Open("Level1BlockSize")->ValueT<u64>());
       }},
      {"Level1",
       [this]() {
         return std::make_shared<ShaList>(
             Level2File(true), Level1File(false),
             Open("Level2BlockSize")->ValueT<u64>());
       }},
      {"Level2",
       [this]() {
         return std::make_shared<ShaList>(
             Level3File(true), Level2File(false),
             Open("Level3BlockSize")->ValueT<u64>());
       }},
      {"Level3",
       [this]() { return std::make_shared<Level3>(Level3File(false)); }},
  });
}

FB::FilePtr Romfs::Level0File() {
  u32 size = Open("Level0Size")->ValueT<u32>();
  return std::make_shared<FB::SubFile>(file, 0x60, size);
}

FB::FilePtr Romfs::Level1Or2File(const std::string &number, bool align_up) {
  u64 l3_size = Open("Level3Size")->ValueT<u64>();
  u64 l3_block_size = Open("Level3BlockSize")->ValueT<u64>();
  l3_size = AlignUp(l3_size, l3_block_size);

  u64 size = Open("Level" + number + "Size")->ValueT<u64>();
  if (align_up) {
    u64 block_size = Open("Level" + number + "BlockSize")->ValueT<u64>();
    size = AlignUp(size, block_size);
  }
  u64 offset = Open("Level" + number + "Offset")->ValueT<u64>();
  return std::make_shared<FB::SubFile>(file, 0x1000 + l3_size + offset, size);
}

FB::FilePtr Romfs::Level1File(bool align_up) {
  return Level1Or2File("1", align_up);
}

FB::FilePtr Romfs::Level2File(bool align_up) {
  return Level1Or2File("2", align_up);
}

FB::FilePtr Romfs::Level3File(bool align_up) {
  u64 size = Open("Level3Size")->ValueT<u64>();
  if (align_up) {
    u64 block_size = Open("Level3BlockSize")->ValueT<u64>();
    size = AlignUp(size, block_size);
  }
  return std::make_shared<FB::SubFile>(file, 0x1000, size);
}

} // namespace CB