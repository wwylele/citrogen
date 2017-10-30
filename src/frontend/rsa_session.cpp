#include "frontend/rsa_session.h"
#include "frontend/util.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <algorithm>

RsaSession::RsaSession(std::shared_ptr<Session> parent_session,
                       const QString &name, CB::ContainerPtr container_)
    : Session(parent_session, name, "RSA-2048 SHA-256 Signature"),
      container(std::move(container_)) {
  QLabel *label_icon = new QLabel();
  QLabel *label_verified = new QLabel();

  CB::ContainerPtr match_file = container->Open("Match");
  if (match_file) {
    bool match = match_file->ValueT<bool>();
    label_icon->setPixmap(match ? *s_pixmap_checked : *s_pixmap_failed);
    label_verified->setText(match ? tr("Signature Verified")
                                  : tr("Signature Mismatch"));
  } else {
    label_verified->setText("Signature Unverified");
  }

  QHBoxLayout *layout_verified = new QHBoxLayout();
  layout_verified->addWidget(label_icon);
  layout_verified->addWidget(label_verified);
  layout_verified->addStretch(1);

  QPlainTextEdit *edit_data = new QPlainTextEdit();
  edit_data->setReadOnly(true);
  edit_data->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  auto signature = container->ValueT<std::vector<u8>>();
  u8 col = 0;
  QString text;
  for (u8 byte : signature) {
    text += ToHex(byte) + " ";
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

QPushButton *RsaSession::CreateButton(Session *parent,
                                      const CB::ContainerPtr &signature,
                                      const std::string &slot) {
  QPushButton *button = new QPushButton(tr("Signature"));
  CB::ContainerPtr match_file = signature->Open("Match");
  if (match_file) {
    bool match = match_file->ValueT<bool>();
    button->setIcon(match ? *s_icon_checked : *s_icon_failed);
    button->setIcon(match ? *s_icon_checked : *s_icon_failed);
  }

  connect(button, &QPushButton::clicked, [parent, signature, slot]() {
    openChildSession(parent, slot, [parent, signature]() {
      return std::make_shared<RsaSession>(parent->shared_from_this(),
                                          tr("Signature"), signature);
    });
  });
  return button;
}