#pragma once

#include "core/container_backend/container.h"
#include "session.h"

class NcchSession : public Session {
  Q_OBJECT
public:
  NcchSession(std::shared_ptr<Session> parent_session, const QString &name,
              CB::ContainerPtr container_);

private:
  void openExheader();
  void openRomfs();
  void openRomfsHashTree();
  void openSmdh();
  CB::ContainerPtr container;
};