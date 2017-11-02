#include "core/file_backend/disk_file.h"
#include <algorithm>
#include <cstdio>
#include <mutex>

// FIXME
#define safe_fseek std::fseek
#define safe_fread std::fread
#define safe_ftell std::ftell

namespace FB {

class DiskFile : public File {
public:
  DiskFile(std::FILE *stream, std::size_t file_size)
      : stream(stream), file_size(file_size) {}
  ~DiskFile() { std::fclose(stream); }

  std::size_t GetSize() override { return file_size; }

  byte_seq Read(std::size_t pos, std::size_t size) override {
    if (pos >= file_size) {
      return {};
    }

    size = std::min(size, file_size - pos);
    byte_seq buffer(size);

    {
      std::lock_guard<std::mutex> lock(stream_mutex);
      safe_fseek(stream, pos, SEEK_SET);
      safe_fread(buffer.data(), size, 1, stream);
    }

    return buffer;
  }

private:
  std::mutex stream_mutex;
  std::FILE *stream;
  std::size_t file_size;
};

FilePtr OpenDiskFile(const std::string &file_name) {
  std::FILE *stream = std::fopen(file_name.c_str(), "rb");
  if (!stream)
    return nullptr;

  // FIXME
  safe_fseek(stream, 0, SEEK_END);
  std::size_t size = safe_ftell(stream);
  safe_fseek(stream, 0, SEEK_SET);

  return std::make_shared<DiskFile>(stream, size);
}

} // namespace FB