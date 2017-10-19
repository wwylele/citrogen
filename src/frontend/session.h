#pragma once

#include <QVBoxLayout>
#include <QWidget>
#include <memory>
#include <string>
#include <unordered_map>

// A Session is a UI interface for a CB::Container, as well as a tab content
class Session : public QWidget, public std::enable_shared_from_this<Session> {
  Q_OBJECT
public:
  Session(std::shared_ptr<Session> parent_session, const QString &name,
          const QString &type);
  QString getName() { return name; }

  static void InitResource();

protected:
  // sub classes should call this in the constructor to setup all the content
  void setupContentLayout(QLayout *layout);

  // sub classes call this to open a child session (when the user hits a button,
  // for example), the function will try to look for an already created session
  // for the same slot. If not, it calls creator_if_not_cached to create it.
  // Finally it emits the signal to ask the tab widget to add it.
  template <typename T>
  void openChildSession(const std::string &slot, T creator_if_not_cached) {
    auto cached = children_session[slot].lock();
    if (!cached) {
      children_session[slot] = cached = creator_if_not_cached();
    }
    emit openSession(cached);
  }

  template <typename T>
  static void openChildSession(Session *session, const std::string &slot,
                               T creator_if_not_cached) {
    session->openChildSession(slot, creator_if_not_cached);
  }

  static QIcon *s_icon_checked;
  static QIcon *s_icon_failed;
  static QPixmap *s_pixmap_checked;
  static QPixmap *s_pixmap_failed;

private:
  std::shared_ptr<Session> parent_session;
  QVBoxLayout *main_layout;
  QString name;
  std::unordered_map<std::string, std::weak_ptr<Session>> children_session;
signals:
  void openSession(std::shared_ptr<Session> session);
};