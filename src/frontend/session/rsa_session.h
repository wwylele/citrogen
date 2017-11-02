#pragma once

#include "core/container_backend/container.h"
#include "session.h"
#include <QPushButton>

class RsaSession : public Session {
  Q_OBJECT
public:
  RsaSession(std::shared_ptr<Session> parent_session, const QString &name,
             CB::ContainerPtr container_);

  static QPushButton *CreateButton(Session *parent,
                                   const CB::ContainerPtr &signature,
                                   const std::string &slot = "sig");

private:
  CB::ContainerPtr container;
};