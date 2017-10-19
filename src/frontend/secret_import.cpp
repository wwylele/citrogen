#include "frontend/secret_import.h"
#include "ui_secret_import.h"

SecretImportDialog::SecretImportDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::SecretImportDialog) {
  ui->setupUi(this);
}

SecretImportDialog::~SecretImportDialog() {}