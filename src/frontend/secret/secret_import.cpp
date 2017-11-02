#include "frontend/secret/secret_import.h"
#include "ui_secret_import.h"
#include <algorithm>

SecretImportDialog::SecretImportDialog(SB::SecretDatabase &&secret_import_,
                                       QWidget *parent)
    : secret_import(std::move(secret_import_)), QDialog(parent),
      ui(new Ui::SecretImportDialog) {
  ui->setupUi(this);
  for (const auto &name : secret_import.List()) {
    ui->listSecret->addItem(QString::fromStdString(name));
  }
  connect(ui->buttonSelectAll, &QPushButton::clicked, this,
          &SecretImportDialog::selectAll);
  selectAll();
}

void SecretImportDialog::selectAll() { ui->listSecret->selectAll(); }

SecretImportDialog::~SecretImportDialog() {}

void SecretImportDialog::accept() {
  int count = ui->listSecret->count();
  auto list = secret_import.List();
  for (auto item : ui->listSecret->selectedItems()) {
    list.erase(std::find(list.begin(), list.end(), item->text().toStdString()));
  }
  for (const auto &name : list) {
    secret_import.Remove(name);
  }
  QDialog::accept();
}