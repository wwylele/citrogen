#include "frontend/format_detect.h"
#include "core/cia.h"
#include "core/ncch.h"
#include "core/ncsd.h"
#include "frontend/cia_session.h"
#include "frontend/ncch_session.h"
#include "frontend/ncsd_session.h"

std::shared_ptr<Session> TryCreateSession(FB::FilePtr file, const QString &name,
                                          std::shared_ptr<Session> parent) {
  auto magic = file->Read(0x100, 4);
  if (magic == std::vector<u8>({'N', 'C', 'S', 'D'})) {
    auto container = std::make_shared<CB::Ncsd>(file);
    return std::make_shared<NcsdSession>(parent, name, container);
  } else if (magic == std::vector<u8>({'N', 'C', 'C', 'H'})) {
    auto container = std::make_shared<CB::Ncch>(file);
    return std::make_shared<NcchSession>(parent, name, container);
  }

  magic = file->Read(0, 4);
  if (magic == std::vector<u8>({0x20, 0x20, 0, 0})) {
    auto container = std::make_shared<CB::Cia>(file);
    return std::make_shared<CiaSession>(parent, name, container);
  }

  return nullptr;
}