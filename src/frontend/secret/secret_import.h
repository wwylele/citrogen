#pragma once

#include "core/secret_backend/secret_database.h"
#include <QDialog>
#include <memory>

namespace Ui {
class SecretImportDialog;
}

class SecretImportDialog : public QDialog {
  Q_OBJECT

public:
  explicit SecretImportDialog(SB::SecretDatabase &&secret_import_,
                              QWidget *parent = 0);
  ~SecretImportDialog();

  SB::SecretDatabase secret_import;

public slots:
  void accept() override;
  void selectAll();

private:
  std::unique_ptr<Ui::SecretImportDialog> ui;
};
