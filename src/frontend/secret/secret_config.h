#pragma once

#include <QDialog>
#include <memory>
#include <string>
#include <unordered_map>

namespace Ui {
class SecretConfigDialog;
}

namespace SB {
class SecretDatabase;
}

class SecretConfigDialog : public QDialog {
  Q_OBJECT

public:
  explicit SecretConfigDialog(std::shared_ptr<SB::SecretDatabase> secrets_,
                              QWidget *parent = 0);
  ~SecretConfigDialog();

  void updateList();

  static std::unordered_map<std::string, QString> secret_desc;

public slots:
  void onManualInputSecret();
  void onRemove();
  void onRemoveAll();
  void onSecretSelected(const QString &name);

private:
  std::shared_ptr<SB::SecretDatabase> secrets;
  std::unique_ptr<Ui::SecretConfigDialog> ui;
};
