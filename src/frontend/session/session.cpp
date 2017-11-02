#include "frontend/session/session.h"
#include <QDebug>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

QIcon *Session::s_icon_checked;
QIcon *Session::s_icon_failed;
QPixmap *Session::s_pixmap_checked;
QPixmap *Session::s_pixmap_failed;

void Session::InitResource() {
  s_icon_checked = new QIcon(":/icons/checked.png");
  s_icon_failed = new QIcon(":/icons/failed.png");
  s_pixmap_checked = new QPixmap(":/icons/checked.png");
  s_pixmap_failed = new QPixmap(":/icons/failed.png");
}

Session::Session(std::shared_ptr<Session> parent_session_, const QString &name_,
                 const QString &type)
    : parent_session(std::move(parent_session_)) {
  if (parent_session) {
    name = parent_session->name + "/" + name_;
  } else {
    name = name_;
  }

  QHBoxLayout *name_bar_layout = new QHBoxLayout();

  QLabel *label_name = new QLabel(name);
  label_name->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
  label_name->setFont(QFontDatabase::systemFont(QFontDatabase::TitleFont));
  name_bar_layout->addWidget(label_name);

  QPushButton *button_open_parent = new QPushButton("Back");
  button_open_parent->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  if (!this->parent_session) {
    button_open_parent->setDisabled(true);
  } else {
    connect(button_open_parent, &QPushButton::clicked,
            [this]() { emit openSession(this->parent_session); });
  }
  name_bar_layout->addWidget(button_open_parent);

  main_layout = new QVBoxLayout();
  main_layout->addLayout(name_bar_layout, 0);

  QLabel *label_type = new QLabel(type);
  label_type->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
  main_layout->addWidget(label_type, 0);

  QFrame *line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  main_layout->addWidget(line);

  setLayout(main_layout);
}

void Session::setupContentLayout(QLayout *layout) {
  main_layout->addLayout(layout, 1);
}