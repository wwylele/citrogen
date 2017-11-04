#include "core/container_backend/disk_directory.h"
#include "core/file_backend/disk_file.h"

#ifdef __GNUC__
#include <experimental/filesystem>
namespace stdfs {
using namespace std::experimental::filesystem;
}
#elif _MSC_VER
#include <filesystem>
namespace stdfs {
using namespace std::experimental::filesystem::v1;
}
#else
#include <filesystem>
namespace stdfs {
using namespace std::filesystem;
}
#endif

namespace CB {

DiskDirectory::DiskDirectory(const std::string &path_string) {
  // TODO: exceptions
  stdfs::path path = stdfs::u8path(path_string);
  stdfs::directory_iterator iter(path);
  for (auto entry : iter) {
    std::string sub_path = entry.path().u8string();
    std::string sub_name = entry.path().filename().u8string();
    if (entry.status().type() == stdfs::file_type::directory) {
      InstallList({{sub_name, [sub_path]() {
                      return std::make_shared<DiskDirectory>(sub_path);
                    }}});
    } else if (entry.status().type() == stdfs::file_type::regular) {
      InstallList({{sub_name, [sub_path]() {
                      return std::make_shared<FileContainer>(
                          FB::OpenDiskFile(sub_path));
                    }}});
    }
  }
}
} // namespace CB