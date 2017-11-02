#include "ncsd_session.h"
#include "frontend/util.h"
#include "ncch_session.h"
#include "rsa_session.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <algorithm>

NcsdSession::NcsdSession(std::shared_ptr<Session> parent_session,
                         const QString &name, CB::ContainerPtr container_)
    : Session(parent_session, name, "NCSD (CCI)"),
      container(std::move(container_)) {
  auto list = container->List();

  QPushButton *button_signature =
      RsaSession::CreateButton(this, container->Open("Signature"));

  u64 media_id = container->Open("MediaId")->ValueT<u64>();
  QPushButton *button_media_id =
      new QPushButton(tr("Media ID\n%1").arg(ToHex(media_id)));

  QVBoxLayout *layout_metadata = new QVBoxLayout();
  layout_metadata->addWidget(button_signature);
  layout_metadata->addWidget(button_media_id);

  QWidget *widget_metadata = new QWidget();
  widget_metadata->setLayout(layout_metadata);

  QScrollArea *area_metadata = new QScrollArea();
  area_metadata->setWidget(widget_metadata);

  QVBoxLayout *layout_partitions = new QVBoxLayout();
  for (std::size_t i = 0; i < 8; ++i) {
    std::string partition_name = CB::WithIndex("Partition", i);
    if (std::find(list.begin(), list.end(), partition_name) != list.end()) {
      QPushButton *button_partition =
          new QPushButton(tr("Partition %1").arg(i));
      connect(button_partition, &QPushButton::clicked,
              [this, i]() { onOpenPartition(i); });
      layout_partitions->addWidget(button_partition);
    }
  }

  QWidget *widget_partitions = new QWidget();
  widget_partitions->setLayout(layout_partitions);

  QScrollArea *area_partitions = new QScrollArea();
  area_partitions->setWidget(widget_partitions);

  QHBoxLayout *main_layout = new QHBoxLayout();
  main_layout->addWidget(area_metadata);
  main_layout->addWidget(area_partitions);

  setupContentLayout(main_layout);
}

void NcsdSession::onOpenPartition(std::size_t index) {
  std::string partition_name = CB::WithIndex("Partition", index);
  openChildSession(std::to_string(index), [this, index, partition_name]() {
    return std::make_shared<NcchSession>(shared_from_this(),
                                         tr("Partition %1").arg(index),
                                         container->Open(partition_name));
  });
}