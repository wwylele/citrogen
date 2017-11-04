#pragma once

#include "core/common_types.h"
#include <array>
#include <vector>

namespace SB {

class Seeddb {
public:
  bool Load(const std::string &file_name);
  bool Save(const std::string &file_name) const;
  byte_seq Get(u64 title_id) const;

private:
  struct Entry {
    u64 title_id;
    std::array<byte, 16> seed;
    byte reserved[8];
  };
  std::vector<Entry> seed_list;
};

extern Seeddb g_seeddb;

} // namespace SB