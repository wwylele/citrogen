#pragma once

#include "frontend/session.h"
#include <QMainWindow>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow();
public slots:
  void onOpen();
  void onManageSecrets();
signals:
  void openNewSession(std::shared_ptr<Session> session);
};