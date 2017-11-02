#include "frontend/format_detect.h"
#include "core/container_backend/cia.h"
#include "core/container_backend/ncch.h"
#include "core/container_backend/ncsd.h"
#include "frontend/session/cia_session.h"
#include "frontend/session/ncch_session.h"
#include "frontend/session/ncsd_session.h"

std::shared_ptr<Session> TryCreateSession(FB::FilePtr file, const QString &name,
                                          std::shared_ptr<Session> parent) {
  auto magic = file->Read<magic_t>(0x100);
  if (magic == magic_t{'N', 'C', 'S', 'D'}) {
    auto container = std::make_shared<CB::Ncsd>(file);
    return std::make_shared<NcsdSession>(parent, name, container);
  } else if (magic == magic_t{'N', 'C', 'C', 'H'}) {
    auto container = std::make_shared<CB::Ncch>(file);
    return std::make_shared<NcchSession>(parent, name, container);
  }

  auto cia_magic = file->Read(0, 4);
  if (cia_magic == byte_seq{byte{0x20}, byte{0x20}, byte{0}, byte{0}}) {
    auto container = std::make_shared<CB::Cia>(file);
    return std::make_shared<CiaSession>(parent, name, container);
  }

  return nullptr;
}