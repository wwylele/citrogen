#include "sha_session.h"
#include "frontend/util.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <algorithm>

ShaSession::ShaSession(std::shared_ptr<Session> parent_session,
                       const QString &name, CB::ContainerPtr container_)
    : Session(parent_session, name, "SHA-256 Hash"),
      container(std::move(container_)) {
  bool match = container->Open("Match")->ValueT<bool>();

  QLabel *label_icon = new QLabel();
  label_icon->setPixmap(match ? *s_pixmap_checked : *s_pixmap_failed);

  QLabel *label_verified = new QLabel();
  label_verified->setText(match ? tr("Hash Verified") : tr("Hash Mismatch"));

  QHBoxLayout *layout_verified = new QHBoxLayout();
  layout_verified->addWidget(label_icon);
  layout_verified->addWidget(label_verified);
  layout_verified->addStretch(1);

  QPlainTextEdit *edit_data = new QPlainTextEdit();
  edit_data->setReadOnly(true);
  edit_data->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  auto signature = container->ValueT<byte_seq>();
  u8 col = 0;
  QString text;
  for (byte b : signature) {
    text += ToHex(b) + " ";
    ++col;
    if (col == 16) {
      text += "\n";
      col = 0;
    }
  }
  edit_data->setPlainText(text);

  QVBoxLayout *main_layout = new QVBoxLayout();
  main_layout->addLayout(layout_verified);
  main_layout->addWidget(edit_data);

  setupContentLayout(main_layout);
}

QPushButton *ShaSession::CreateButton(Session *parent,
                                      const CB::ContainerPtr &signature,
                                      const std::string &slot,
                                      const QString &name) {
  QPushButton *button = new QPushButton(name);
  bool match = signature->Open("Match")->ValueT<bool>();
  button->setIcon(match ? *s_icon_checked : *s_icon_failed);
  connect(button, &QPushButton::clicked, [parent, signature, slot, name]() {
    openChildSession(parent, slot, [parent, signature, name]() {
      return std::make_shared<ShaSession>(parent->shared_from_this(), name,
                                          signature);
    });
  });
  return button;
}
