#pragma once

#include "core/container.h"
#include "frontend/session.h"
#include <QAction>
#include <QPushButton>

class ShaSession : public Session {
  Q_OBJECT
public:
  ShaSession(std::shared_ptr<Session> parent_session, const QString &name,
             CB::ContainerPtr container_);

  static QPushButton *CreateButton(Session *parent,
                                   const CB::ContainerPtr &signature,
                                   const std::string &slot,
                                   const QString &name);

private:
  CB::ContainerPtr container;
};