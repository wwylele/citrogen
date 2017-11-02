#pragma once

#include "core/common_types.h"
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

namespace FB {

class File {
public:
  File();

  File(const File &) = delete;

  File &operator=(const File &) = delete;

  virtual ~File();

  virtual std::size_t GetSize() = 0;

  virtual byte_seq Read(std::size_t pos, std::size_t size) = 0;

  template <typename T> T Read(std::size_t pos) {
    static_assert(std::is_trivially_copyable<T>::value,
                  "T must be trivially copyable!");
    T t;
    auto raw = Read(pos, sizeof(T));
    std::memcpy(&t, raw.data(), sizeof(T));
    return t;
  }
};

using FilePtr = std::shared_ptr<File>;

} // namespace FB
