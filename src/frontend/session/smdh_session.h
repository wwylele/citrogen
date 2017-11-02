#pragma once

#include "core/container_backend/container.h"
#include "frontend/session/session.h"

class SmdhSession : public Session {
  Q_OBJECT
public:
  SmdhSession(std::shared_ptr<Session> parent_session, const QString &name,
              CB::ContainerPtr container_,
              const CB::ContainerPtr &hash_container);

private:
  CB::ContainerPtr container;
};