#include "frontend/smdh_session.h"
#include "frontend/sha_session.h"
#include "frontend/util.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>

SmdhSession::SmdhSession(std::shared_ptr<Session> parent_session,
                         const QString &name, CB::ContainerPtr container_,
                         const CB::ContainerPtr &hash_container)
    : Session(parent_session, name, "SMDH (Icon File)"),
      container(std::move(container_)) {
  QPushButton *hash_button =
      ShaSession::CreateButton(this, hash_container, "hash", tr("Hash"));
  QHBoxLayout *layout_hash = new QHBoxLayout();
  layout_hash->addWidget(hash_button);
  layout_hash->addStretch(1);

  auto small = container->Open("IconSmall")->ValueT<std::array<u16, 24 * 24>>();
  auto large = container->Open("IconLarge")->ValueT<std::array<u16, 48 * 48>>();
  QImage icon_small(reinterpret_cast<const uchar *>(small.data()), 24, 24,
                    QImage::Format::Format_RGB16);
  QImage icon_large(reinterpret_cast<const uchar *>(large.data()), 48, 48,
                    QImage::Format::Format_RGB16);

  QHBoxLayout *layout_icon = new QHBoxLayout();
  layout_icon->addWidget(new QLabel(tr("Icon:")));
  QLabel *label_large = new QLabel();
  QLabel *label_small = new QLabel();
  label_large->setPixmap(QPixmap::fromImage(icon_large));
  label_small->setPixmap(QPixmap::fromImage(icon_small));
  layout_icon->addWidget(label_large);
  layout_icon->addWidget(label_small);
  layout_icon->addStretch(1);

  QTableWidget *table_title = new QTableWidget();
  table_title->verticalHeader()->hide();
  table_title->setColumnCount(3);
  table_title->setHorizontalHeaderLabels(
      {tr("Long Title"), tr("Short Title"), tr("Publisher")});
  table_title->setRowCount(13);

  QVBoxLayout *main_layout = new QVBoxLayout();
  main_layout->addLayout(layout_hash);
  main_layout->addLayout(layout_icon);
  main_layout->addWidget(table_title);

  setupContentLayout(main_layout);
}