#include "core/secret_database.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace SB {

static std::shared_ptr<const SecretDatabase> g_secrets;
static std::string g_file_name;

char k_magic[] = "citrogendatabase";

bool SecretDatabase::Load(const std::string &file_name) {
  database.clear();
  std::unique_ptr<std::FILE, decltype(&std::fclose)> file(
      std::fopen(file_name.c_str(), "rb"), &std::fclose);
  if (!file)
    return false;
  char magic[16];
  std::fread(magic, 16, 1, file.get());
  if (std::memcmp(magic, k_magic, 16) != 0)
    return false;
  u64 size;
  std::fread(&size, sizeof(u64), 1, file.get());
  for (u64 i = 0; i < size; ++i) {
    u64 name_size, value_size;
    std::fread(&name_size, sizeof(u64), 1, file.get());
    std::string name(name_size, ' ');
    std::fread(name.data(), name_size, 1, file.get());
    std::fread(&value_size, sizeof(u64), 1, file.get());
    std::vector<u8> value(value_size);
    std::fread(value.data(), value_size, 1, file.get());
    database.emplace(name, value);
  }
  return true;
}

bool SecretDatabase::Save(const std::string &file_name) const {
  std::unique_ptr<std::FILE, decltype(&std::fclose)> file(
      std::fopen(file_name.c_str(), "wb"), &std::fclose);
  if (!file)
    return false;
  std::fwrite(k_magic, 16, 1, file.get());
  u64 size = database.size();
  std::fwrite(&size, sizeof(u64), 1, file.get());
  for (const auto & [ name, value ] : database) {
    u64 name_size = name.size(), value_size = value.size();
    std::fwrite(&name_size, sizeof(u64), 1, file.get());
    std::fwrite(name.data(), name_size, 1, file.get());
    std::fwrite(&value_size, sizeof(u64), 1, file.get());
    std::fwrite(value.data(), value_size, 1, file.get());
  }
  return true;
}

std::vector<u8> SecretDatabase::Get(const std::string &name) const {
  auto found = database.find(name);
  if (found == database.end())
    return {};
  return found->second;
}

void SecretDatabase::Set(const std::string &name,
                         const std::vector<u8> &value) {
  database[name] = value;
}

void SecretDatabase::Remove(const std::string &name) { database.erase(name); }

void SecretDatabase::RemoveAll() { database.clear(); }

std::vector<std::string> SecretDatabase::List() const {
  std::vector<std::string> result(database.size());
  std::transform(database.begin(), database.end(), result.begin(),
                 [](const auto &i) { return i.first; });
  return result;
}

SecretContext::SecretContext() : database(g_secrets) {}

std::vector<u8> SecretContext::operator[](const std::string &name) const {
  return database->Get(name);
}

void Init(const std::string &file_name) {
  g_file_name = file_name;
  auto new_secrets = std::make_shared<SecretDatabase>();
  new_secrets->Load(g_file_name);
  g_secrets = new_secrets;
}

std::shared_ptr<SecretDatabase> Lock() {
  return std::make_shared<SecretDatabase>(*g_secrets);
}

void Unlock(std::shared_ptr<SecretDatabase> &&database) {
  g_secrets = std::move(database);
  g_secrets->Save(g_file_name);
}

void Discard(std::shared_ptr<SecretDatabase> &&database) {}

} // namespace SB