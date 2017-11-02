#pragma once

#include "file.h"
#include <string>

namespace FB {

FilePtr OpenDiskFile(const std::string &file_name);

} // namespace FB