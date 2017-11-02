#include "frontend/secret/secret_input.h"
#include "frontend/secret/secret_config.h"
#include "frontend/util.h"
#include "ui_secret_input.h"
#include <algorithm>
#include <list>

SecretInputDialog::SecretInputDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::SecretInputDialog) {
  ui->setupUi(this);
  std::list<std::string> name_list(SecretConfigDialog::secret_desc.size());
  std::transform(SecretConfigDialog::secret_desc.begin(),
                 SecretConfigDialog::secret_desc.end(), name_list.begin(),
                 [](const auto &a) { return a.first; });
  name_list.sort();
  for (const auto &name : name_list) {
    ui->comboSecretName->addItem(QString::fromStdString(name));
  }
}

SecretInputDialog::~SecretInputDialog() {}

void SecretInputDialog::accept() {
  return_name = ui->comboSecretName->currentText().toStdString();
  u8 temp = 0;
  bool low = false;

  // TODO refactor this
  for (QChar c : ui->editSecretValue->toPlainText()) {
    int digit = DigitFromHex(c);
    if (digit < 0)
      continue;
    if (!low) {
      temp = (u8)(digit << 4);
    } else {
      temp |= (u8)digit;
      return_value.push_back(byte{temp});
    }

    low = !low;
  }

  QDialog::accept();
}