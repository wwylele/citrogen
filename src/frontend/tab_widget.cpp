#include "frontend/tab_widget.h"
#include "frontend/ncsd_session.h"
#include <QApplication>
#include <QDebug>
#include <QDrag>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QVBoxLayout>

TabBar::TabBar(QWidget *parent) : QTabBar(parent) {
  setContextMenuPolicy(Qt::CustomContextMenu);
  setAcceptDrops(true);
  connect(this, &TabBar::customContextMenuRequested, this,
          &TabBar::onContextMenuRequested);

  setTabsClosable(true);
  connect(this, &TabBar::tabCloseRequested, this, &TabBar::closeTab);
  setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
  setMovable(true);
}

void TabBar::onContextMenuRequested(const QPoint &position) {
  QMenu menu;
  int index = tabAt(position);
  if (-1 != index) {
    QAction *action =
        menu.addAction(tr("&Close Tab"), this, SLOT(onCloseTab()));
    action->setData(index);

    action =
        menu.addAction(tr("Close &Other Tabs"), this, SLOT(onCloseOtherTabs()));
    action->setData(index);
  } else {
    menu.addSeparator();
  }
  menu.exec(QCursor::pos());
}

void TabBar::onCloseTab() {
  if (QAction *action = qobject_cast<QAction *>(sender())) {
    int index = action->data().toInt();
    emit closeTab(index);
  }
}

void TabBar::onCloseOtherTabs() {
  if (QAction *action = qobject_cast<QAction *>(sender())) {
    int index = action->data().toInt();
    emit closeOtherTabs(index);
  }
}

/////////////////////////////////

SessionContainer::SessionContainer(std::shared_ptr<Session> session,
                                   QWidget *parent)
    : QWidget(parent), session(std::move(session)) {
  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(this->session.get());
  setLayout(layout);
}

SessionContainer::~SessionContainer() {
  session->setParent(nullptr); // deparent from the layout
}

/////////////////////////////////

TabWidget::TabWidget(QWidget *parent) : QTabWidget(parent) {
  setElideMode(Qt::ElideRight);

  TabBar *tab_bar = new TabBar(this);
  connect(tab_bar, &TabBar::closeTab, this, &TabWidget::closeTab);
  connect(tab_bar, &TabBar::closeOtherTabs, this, &TabWidget::closeOtherTabs);

  setTabBar(tab_bar);
  setDocumentMode(true);
}

void TabWidget::addSession(std::shared_ptr<Session> session) {
  connect(session.get(), &Session::openSession, this, &TabWidget::openSession);
  setCurrentIndex(addTab(new SessionContainer(session), session->getName()));
}

void TabWidget::openSession(std::shared_ptr<Session> session) {
  int c = count();
  for (int i = 0; i < c; ++i) {
    SessionContainer *container = qobject_cast<SessionContainer *>(widget(i));
    if (container->session == session) {
      setCurrentIndex(i);
      return;
    }
  }
  addSession(session);
}

void TabWidget::closeOtherTabs(int index) {
  if (-1 == index)
    return;
  for (int i = count() - 1; i > index; --i)
    closeTab(i);
  for (int i = index - 1; i >= 0; --i)
    closeTab(i);
}

void TabWidget::closeTab(int index) {
  if (index < 0)
    index = currentIndex();
  if (index < 0 || index >= count())
    return;

  auto *w = widget(index);
  removeTab(index);
  delete w;
}
