#include "frontend/ncch_session.h"
#include "frontend/exheader_session.h"
#include "frontend/romfs_hash_session.h"
#include "frontend/romfs_session.h"
#include "frontend/rsa_session.h"
#include "frontend/sha_session.h"
#include "frontend/smdh_session.h"
#include "frontend/util.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <algorithm>

NcchSession::NcchSession(std::shared_ptr<Session> parent_session,
                         const QString &name, CB::ContainerPtr container_)
    : Session(parent_session, name, "NCCH (CXI or CFA)"),
      container(std::move(container_)) {
  auto list = container->List();

  QPushButton *button_signature =
      RsaSession::CreateButton(this, container->Open("Signature"));
  QPushButton *button_signature_patched =
      RsaSession::CreateButton(this, container->Open("SignaturePatched"));
  button_signature_patched->setText(tr("Signature(patched)"));

  u64 partition_id = container->Open("PartitionId")->ValueT<u64>();
  QPushButton *button_partition_id =
      new QPushButton(tr("Partition ID\n%1").arg(ToHex(partition_id)));

  u64 program_id = container->Open("ProgramId")->ValueT<u64>();
  QPushButton *button_program_id =
      new QPushButton(tr("Program ID\n%1").arg(ToHex(program_id)));

  auto product_code =
      container->Open("ProductCode")->ValueT<std::array<char, 0x10>>();
  QPushButton *button_product_code =
      new QPushButton(tr("Product Code\n%1")
                          .arg(QString::fromLatin1(product_code.data(), 0x10)));

  QString content_flags = tr("Content Type Flags:\n");
  if (container->Open("IsData")->ValueT<bool>())
    content_flags += tr("Data\n");
  if (container->Open("IsExecutable")->ValueT<bool>())
    content_flags += tr("Executable\n");

  QString content_type_desc;
  u8 content_type = container->Open("ContentType")->ValueT<u8>();
  switch (content_type) {
  case 0:
    content_type_desc = tr("Application\n");
    break;
  case 1:
    content_type_desc = tr("System Update\n");
    break;
  case 2:
    content_type_desc = tr("Manual\n");
    break;
  case 3:
    content_type_desc = tr("Child\n");
    break;
  case 4:
    content_type_desc = tr("Trial\n");
    break;
  case 5:
    content_type_desc = tr("Extended System Update\n");
    break;
  default:
    content_type_desc = tr("Unknown Content Type %1\n").arg(content_type);
    break;
  }

  content_flags += content_type_desc;

  if (container->Open("IsNoRomfsMount")->ValueT<bool>())
    content_flags += tr("No RomFS Mount\n");
  u8 platform = container->Open("Platform")->ValueT<u8>();
  if (platform == 2) {
    content_flags += tr("New 3DS Exclusive\n");
  } else if (platform != 1) {
    content_flags += tr("Unknown Platform %1\n").arg(platform);
  }

  QLabel *label_content_flags = new QLabel(content_flags);

  QString crypto_type = tr("Crypto Type:\n");
  u8 method = container->Open("CryptoMethod")->ValueT<u8>();
  switch (method) {
  case 0x00:
    crypto_type += tr("Standard");
    break;
  case 0x01:
    crypto_type += tr("7.x");
    break;
  case 0x0A:
    crypto_type += tr("Secure3");
    break;
  case 0x0B:
    crypto_type += tr("Secure4");
    break;
  default:
    crypto_type += tr("Unknown (%1)").arg(method);
    break;
  }

  if (container->Open("IsFixedKeyCrypto")->ValueT<bool>()) {
    crypto_type += tr(", Fixed Key");
  }

  if (container->Open("IsSeedCrypto")->ValueT<bool>()) {
    crypto_type += tr(", Seed Crypto");
  }

  if (container->Open("IsNoCrypto")->ValueT<bool>()) {
    crypto_type += tr(", No Crypto");
  }

  if (container->Open("IsForceNoCrypto")->ValueT<bool>()) {
    crypto_type += tr("\nActually Decrypted");
  }

  QLabel *label_crypto = new QLabel(crypto_type);

  QVBoxLayout *layout_metadata = new QVBoxLayout();
  layout_metadata->addWidget(button_signature);
  layout_metadata->addWidget(button_signature_patched);
  layout_metadata->addWidget(button_partition_id);
  layout_metadata->addWidget(button_program_id);
  layout_metadata->addWidget(button_product_code);
  layout_metadata->addWidget(label_content_flags);
  layout_metadata->addWidget(label_crypto);

  QWidget *widget_metadata = new QWidget();
  widget_metadata->setLayout(layout_metadata);

  QScrollArea *area_metadata = new QScrollArea();
  area_metadata->setWidget(widget_metadata);

  QGridLayout *layout_partitions = new QGridLayout();

  // TODO: don't use "error" field to check existence
  if (std::find(list.begin(), list.end(), "ExheaderError") != list.end()) {
    auto error = container->Open("ExheaderError")->ValueT<std::string>();
    if (error.empty()) {
      QPushButton *button_exheader = new QPushButton("Exheader");
      connect(button_exheader, &QPushButton::clicked, this,
              &NcchSession::openExheader);
      layout_partitions->addWidget(button_exheader, 0, 0);

      QPushButton *button_exheader_hash =
          ShaSession::CreateButton(this, container->Open("ExheaderHash"),
                                   "hash_exheader", tr("Exheader Hash"));
      layout_partitions->addWidget(button_exheader_hash, 0, 1);
    } else {
      // TODO: show proper information when encrypted
      layout_partitions->addWidget(new QLabel(QString::fromStdString(error)), 0,
                                   0);
    }
  }

  if (std::find(list.begin(), list.end(), "ExefsError") != list.end()) {
    auto error = container->Open("ExefsError")->ValueT<std::string>();
    if (error.empty()) {
      QPushButton *button_exefs = new QPushButton("ExeFS");
      QMenu *menu = new QMenu();
      auto exefs = container->Open("Exefs");
      auto list = exefs->List();
      for (const auto &name : list) {
        if (name.substr(0, 5) == "Hash:")
          continue;
        QAction *action = menu->addAction(QString::fromStdString(name));
        if (name == "icon") {
          connect(action, &QAction::triggered, this, &NcchSession::openSmdh);
        }
      }
      button_exefs->setMenu(menu);
      layout_partitions->addWidget(button_exefs, 1, 0);

      QPushButton *button_exefs_hash = ShaSession::CreateButton(
          this, container->Open("ExefsHash"), "hash_exefs", tr("ExeFS Hash"));
      layout_partitions->addWidget(button_exefs_hash, 1, 1);
    } else {
      layout_partitions->addWidget(new QLabel(QString::fromStdString(error)), 1,
                                   0);
    }
  }

  if (std::find(list.begin(), list.end(), "RomfsError") != list.end()) {
    auto error = container->Open("RomfsError")->ValueT<std::string>();
    if (error.empty()) {
      QPushButton *button_romfs = new QPushButton("RomFS");
      QMenu *menu = new QMenu();
      connect(menu->addAction(tr("Content")), &QAction::triggered, this,
              &NcchSession::openRomfs);
      connect(menu->addAction(tr("Hash Tree")), &QAction::triggered, this,
              &NcchSession::openRomfsHashTree);
      button_romfs->setMenu(menu);
      layout_partitions->addWidget(button_romfs, 2, 0);

      QPushButton *button_romfs_hash = ShaSession::CreateButton(
          this, container->Open("RomfsHash"), "hash_romfs", tr("RomFS Hash"));
      layout_partitions->addWidget(button_romfs_hash, 2, 1);
    } else {
      layout_partitions->addWidget(new QLabel(QString::fromStdString(error)), 2,
                                   0);
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

void NcchSession::openExheader() {
  openChildSession("exheader", [this]() {
    return std::make_shared<ExheaderSession>(shared_from_this(), tr("Exheader"),
                                             container->Open("Exheader"));
  });
}

void NcchSession::openRomfs() {
  openChildSession("romfs", [this]() {
    return std::make_shared<RomfsSession>(
        shared_from_this(), tr("RomFS"),
        container->Open("Romfs")->Open("Level3")->Open("."));
  });
}

void NcchSession::openRomfsHashTree() {
  openChildSession("romfsh", [this]() {
    return std::make_shared<RomfsHashSession>(
        shared_from_this(), tr("RomFS Hash"), container->Open("Romfs"));
  });
}

void NcchSession::openSmdh() {
  openChildSession("icon", [this]() {
    return std::make_shared<SmdhSession>(
        shared_from_this(), tr("icon"), container->Open("Exefs")->Open("icon"),
        container->Open("Exefs")->Open("Hash:icon"));
  });
}