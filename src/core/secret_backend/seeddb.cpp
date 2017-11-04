#include "core/secret_backend/seeddb.h"
#include <algorithm>
#include <cstdio>
#include <memory>

namespace SB {

Seeddb g_seeddb;

bool Seeddb::Load(const std::string &file_name) {
  std::unique_ptr<std::FILE, decltype(&std::fclose)> file(
      std::fopen(file_name.c_str(), "rb"), &std::fclose);
  if (!file)
    return false;

  u32 num_entries;
  std::fread(&num_entries, sizeof(u32), 1, file.get());
  std::fseek(file.get(), 12, SEEK_CUR);
  seed_list.resize(num_entries);
  std::fread(seed_list.data(), sizeof(Entry), num_entries, file.get());
}

bool Seeddb::Save(const std::string &file_name) const {
  std::unique_ptr<std::FILE, decltype(&std::fclose)> file(
      std::fopen(file_name.c_str(), "wb"), &std::fclose);
  if (!file)
    return false;

  u32 num_entries = static_cast<u32>(seed_list.size());
  std::fwrite(&num_entries, sizeof(u32), 1, file.get());
  std::fseek(file.get(), 12, SEEK_CUR);
  std::fwrite(seed_list.data(), sizeof(Entry), num_entries, file.get());
}

byte_seq Seeddb::Get(u64 title_id) const {
  auto found = std::find_if(
      seed_list.begin(), seed_list.end(),
      [title_id](const auto &entry) { return entry.title_id == title_id; });
  if (found == seed_list.end())
    return {};
  return byte_seq(found->seed.begin(), found->seed.end());
}

} // namespace SB