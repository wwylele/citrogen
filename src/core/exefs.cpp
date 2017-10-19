#include "core/exefs.h"
#include "core/sha.h"
#include "core/smdh.h"
#include <iostream>
namespace CB {

Exefs::Exefs(FB::FilePtr primary_, FB::FilePtr secondary_)
    : primary(std::move(primary_)), secondary(std::move(secondary_)) {
  struct FileHeader {
    std::array<char, 8> name;
    u32 offset;
    u32 size;
  };
  for (unsigned i = 0; i < 10; ++i) {
    auto header = primary->Read<FileHeader>(i * sizeof(FileHeader));
    std::string name(header.name.begin(), header.name.end());
    auto pos = name.find('\0');
    if (pos != std::string::npos) {
      name.erase(pos);
    }

    if (name == "")
      continue;

    FB::FilePtr overlay_file;
    if (name == "icon" || name == "banner") {
      overlay_file = primary;
    } else {
      overlay_file = secondary;
    }

    FB::FilePtr sub_file = std::make_shared<FB::SubFile>(
        overlay_file, 0x200 + header.offset, header.size);
    FB::FilePtr hash_file =
        std::make_shared<FB::SubFile>(primary, 0xC0 + (9 - i) * 0x20, 0x20);

    if (name == "icon") {
      InstallList(
          {{name, [sub_file]() { return std::make_shared<Smdh>(sub_file); }}});
    } else if (name == "banner") {
    } else if (name == "logo") {
    } else if (name == ".code") {
    } else
      std::cout << name << std::endl;

    // InstallList({{name, [](){
    //    return nullptr;
    //}}});

    InstallList({{"Hash:" + name, [sub_file, hash_file]() {
                    return std::make_shared<Sha>(sub_file, hash_file);
                  }}});
  }
}

} // namespace CB
