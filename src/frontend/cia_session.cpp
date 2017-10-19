#include "frontend/cia_session.h"
#include "frontend/ncch_session.h"
#include "frontend/sha_session.h"
#include "frontend/util.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <algorithm>

CiaSession::CiaSession(std::shared_ptr<Session> parent_session,
                       const QString &name, CB::ContainerPtr container_)
    : Session(parent_session, name, "CIA (CTR Importable Archive)"),
      container(std::move(container_)) {
  QGridLayout *layout_partitions = new QGridLayout();

  auto tmd = container->Open("Tmd");
  u16 content_count = tmd->Open("ContentCount")->ValueT<u16>();
  for (u16 i = 0; i < content_count; ++i) {
    std::string error = container->Open(CB::WithIndex("ContentError", i))
                            ->ValueT<std::string>();
    if (error.empty()) {
      QPushButton *button_content = new QPushButton(tr("Content %1").arg(i));
      connect(button_content, &QPushButton::clicked,
              [this, i]() { onOpenContent(i); });
      layout_partitions->addWidget(button_content, i, 0);

      QPushButton *button_hash = ShaSession::CreateButton(
          this, container->Open(CB::WithIndex("ContentHash", i)),
          "hash" + std::to_string(i), tr("Content %1 Hash").arg(i));
      layout_partitions->addWidget(button_hash, i, 1);
    } else {
      // TODO
      layout_partitions->addWidget(new QLabel(QString::fromStdString(error)), i,
                                   0);
    }

    u32 id = tmd->Open(CB::WithIndex("ContentId", i))->ValueT<u32>();
    u16 index = tmd->Open(CB::WithIndex("ContentIndex", i))->ValueT<u16>();
    bool is_encrypted =
        tmd->Open(CB::WithIndex("ContentIsEncrypted", i))->ValueT<bool>();
    bool is_disc = tmd->Open(CB::WithIndex("ContentIsDisc", i))->ValueT<bool>();
    bool is_cfm = tmd->Open(CB::WithIndex("ContentIsCfm", i))->ValueT<bool>();
    bool is_optional =
        tmd->Open(CB::WithIndex("ContentIsOptional", i))->ValueT<bool>();
    bool is_shared =
        tmd->Open(CB::WithIndex("ContentIsShared", i))->ValueT<bool>();
    QString info = tr("id = 0x%1, index = %2").arg(ToHex(id)).arg(index);
    if (is_encrypted)
      info += tr(", encrypted");
    if (is_disc)
      info += tr(", disc");
    if (is_cfm)
      info += tr(", cfm");
    if (is_optional)
      info += tr(", optional");
    if (is_shared)
      info += tr(", shared");

    layout_partitions->addWidget(new QLabel(info), i, 2);
  }

  QWidget *widget_partitions = new QWidget();
  widget_partitions->setLayout(layout_partitions);

  QScrollArea *area_partitions = new QScrollArea();
  area_partitions->setWidget(widget_partitions);

  QHBoxLayout *main_layout = new QHBoxLayout();
  main_layout->addWidget(area_partitions);

  setupContentLayout(main_layout);
}

void CiaSession::onOpenContent(std::size_t index) {
  std::string partition_name = CB::WithIndex("Content", index);
  openChildSession(std::to_string(index), [this, index, partition_name]() {
    return std::make_shared<NcchSession>(shared_from_this(),
                                         tr("Content %1").arg(index),
                                         container->Open(partition_name));
  });
}
