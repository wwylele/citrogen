#pragma once

#include <QDialog>
#include <memory>

namespace Ui {
class SecretImportDialog;
}

class SecretImportDialog : public QDialog {
  Q_OBJECT

public:
  explicit SecretImportDialog(QWidget *parent = 0);
  ~SecretImportDialog();

private:
  std::unique_ptr<Ui::SecretImportDialog> ui;
};
