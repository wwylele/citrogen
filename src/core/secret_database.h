#pragma once

#include "core/common_types.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace SB {

class SecretDatabase {
public:
  bool Load(const std::string &file_name);
  bool Save(const std::string &file_name) const;
  std::vector<u8> Get(const std::string &name) const;
  void Set(const std::string &name, const std::vector<u8> &value);
  void Remove(const std::string &name);
  void RemoveAll();
  std::vector<std::string> List() const;

private:
  std::unordered_map<std::string, std::vector<u8>> database;
};

class SecretContext {
public:
  SecretContext();
  std::vector<u8> operator[](const std::string &name) const;

private:
  std::shared_ptr<const SecretDatabase> database;
};

void Init(const std::string &file_name);
std::shared_ptr<SecretDatabase> Lock();
void Unlock(std::shared_ptr<SecretDatabase> &&database);
void Discard(std::shared_ptr<SecretDatabase> &&database);

const std::string k_sec_key3D_x = "AES Slot 0x3D Key X";
const std::string k_sec_key3D_y[6] = {
    "AES Slot 0x3D Key Y0", "AES Slot 0x3D Key Y1", "AES Slot 0x3D Key Y2",
    "AES Slot 0x3D Key Y3", "AES Slot 0x3D Key Y4", "AES Slot 0x3D Key Y5",
};
const std::string k_sec_key2C_x = "AES Slot 0x2C Key X";
const std::string k_sec_key25_x = "AES Slot 0x25 Key X";
const std::string k_sec_key18_x = "AES Slot 0x18 Key X";
const std::string k_sec_key1B_x = "AES Slot 0x1B Key X";
const std::string k_sec_aes_const = "AES Key Scrambler Constant";
const std::string k_sec_pubkey_exheader = "ExHeader Signature Public Key";
const std::string k_sec_pubkey_ncsd_cfa = "NCSD and CFA Signature Public Key";

} // namespace SB