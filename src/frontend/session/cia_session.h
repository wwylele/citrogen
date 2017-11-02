#pragma once

#include "core/container_backend/container.h"
#include "frontend/session/session.h"

class CiaSession : public Session {
  Q_OBJECT
public:
  CiaSession(std::shared_ptr<Session> parent_session, const QString &name,
             CB::ContainerPtr container_);
  void onOpenContent(std::size_t index);

private:
  CB::ContainerPtr container;
};