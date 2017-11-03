#pragma once

#include "frontend/session/session.h"
#include <QMainWindow>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow();
public slots:
  void onOpen();
  void onOpenSd();
  void onManageSecrets();
signals:
  void openNewSession(std::shared_ptr<Session> session);
};