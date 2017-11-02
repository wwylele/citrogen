#pragma once

#include "core/container_backend/container.h"
#include "frontend/session/session.h"

class ExheaderSession : public Session {
  Q_OBJECT
public:
  ExheaderSession(std::shared_ptr<Session> parent_session, const QString &name,
                  CB::ContainerPtr container_);

private:
  CB::ContainerPtr container;
};