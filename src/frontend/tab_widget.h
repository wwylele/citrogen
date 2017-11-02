#pragma once

#include "session/session.h"
#include <QTabBar>
#include <QTabWidget>
#include <memory>

class TabBar : public QTabBar {
  Q_OBJECT

signals:
  void closeTab(int index);
  void closeOtherTabs(int index);

public:
  TabBar(QWidget *parent = 0);

private slots:
  void onCloseTab();
  void onCloseOtherTabs();
  void onContextMenuRequested(const QPoint &position);

  friend class TabWidget;
};

// This is a wrapper to defeat Qt's widget ownership. It owns the Session widget
// by std::shared_ptr ownership.
class SessionContainer : public QWidget {
  Q_OBJECT

public:
  SessionContainer(std::shared_ptr<Session> session, QWidget *parent = 0);
  ~SessionContainer();

private:
  std::shared_ptr<Session> session;
  friend class TabWidget;
};

class TabWidget : public QTabWidget {
  Q_OBJECT

public:
  TabWidget(QWidget *parent = 0);

public slots:

  // create a new tab for the given session
  void addSession(std::shared_ptr<Session> session);

  // go to the tab for the given session. create one if not exists
  void openSession(std::shared_ptr<Session> session);

  void closeTab(int index = -1);
  void closeOtherTabs(int index);
};
