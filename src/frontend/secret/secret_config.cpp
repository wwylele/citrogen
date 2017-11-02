#include "secret_config.h"
#include "core/secret_backend/bootrom.h"
#include "core/secret_backend/secret_database.h"
#include "frontend/util.h"
#include "secret_input.h"
#include "ui_secret_config.h"
#include <QMenu>

std::unordered_map<std::string, QString> SecretConfigDialog::secret_desc;

SecretConfigDialog::SecretConfigDialog(
    std::shared_ptr<SB::SecretDatabase> secrets_, QWidget *parent)
    : QDialog(parent), secrets(std::move(secrets_)),
      ui(new Ui::SecretConfigDialog) {
  ui->setupUi(this);

  // HACK
  secret_desc = {
      {SB::k_sec_key2C_x,
       tr("The primary encryption key for NCCH. All encrypted NCCH files, "
          "except "
          "for ones using fixed-key crypto, need this key to decrypt.")},
      {SB::k_sec_key25_x, tr("The secondary encryption key for 7.x (Secure 2) "
                             "NCCH. Many encrypted "
                             "NCCH files need this key to decrypt.")},
      {SB::k_sec_key18_x,
       tr("The secondary encryption key for Secure 3 NCCH. Some encrypted NCCH "
          "files need this key to decrypt.")},
      {SB::k_sec_key1B_x,
       tr("The secondary encryption key for Secure 4 NCCH. Some encrypted NCCH "
          "files need this key to decrypt.")},
      {SB::k_sec_key3D_x,
       tr("The primary key for decrypting ticket title key.")},
      {SB::k_sec_key3D_y[0], tr("The secondary key for decrypting ticket title "
                                "key used for eshop applications.")},
      {SB::k_sec_key3D_y[1], tr("The secondary key for decrypting ticket title "
                                "key used for system applications.")},
      {SB::k_sec_aes_const, tr("The core secret constant of AES key scrambler "
                               "engine. Needed for most AES encryption.")},
      {SB::k_sec_pubkey_exheader,
       tr("The public key needed for verifying ExHeader signature.")},
      {SB::k_sec_pubkey_ncsd_cfa,
       tr("The public key needed for verifying NCSD and CFA signature.")},
  };

  QMenu *menu = new QMenu();
  connect(menu->addAction(tr("Manual Input...")), &QAction::triggered, this,
          &SecretConfigDialog::onManualInputSecret);
  ui->buttonImport->setMenu(menu);

  connect(ui->listSecret, &QListWidget::currentTextChanged, this,
          &SecretConfigDialog::onSecretSelected);
  connect(ui->buttonRemove, &QPushButton::clicked, this,
          &SecretConfigDialog::onRemove);
  connect(ui->buttonRemoveAll, &QPushButton::clicked, this,
          &SecretConfigDialog::onRemoveAll);

  updateList();
}

SecretConfigDialog::~SecretConfigDialog() {}

void SecretConfigDialog::onSecretSelected(const QString &name) {
  std::string std_name = name.toStdString();

  QString desc_string = "";
  auto desc = secret_desc.find(std_name);
  if (desc != secret_desc.end()) {
    desc_string = desc->second;
  }
  ui->editDescription->setPlainText(desc_string);

  QString value_str;
  for (byte b : secrets->Get(std_name))
    value_str += ToHex(b);

  ui->editValue->setPlainText(value_str);
}

void SecretConfigDialog::updateList() {
  ui->listSecret->clear();
  for (const std::string &name : secrets->List()) {
    ui->listSecret->addItem(QString::fromStdString(name));
  }
}

void SecretConfigDialog::onRemove() {
  auto item = ui->listSecret->currentItem();
  if (item == nullptr)
    return;
  secrets->Remove(item->text().toStdString());
  updateList();
}

void SecretConfigDialog::onRemoveAll() {
  ui->listSecret->clear();
  secrets->RemoveAll();
}

void SecretConfigDialog::onManualInputSecret() {
  SecretInputDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected)
    return;
  secrets->Set(dialog.return_name, dialog.return_value);
  updateList();
  auto item = ui->listSecret->findItems(
      QString::fromStdString(dialog.return_name), Qt::MatchExactly)[0];
  ui->listSecret->setCurrentItem(item);
}