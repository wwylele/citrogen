#include "core/common_types.h"
#include <QDialog>
#include <memory>
#include <string>
#include <vector>

namespace Ui {
class SecretInputDialog;
}

class SecretInputDialog : public QDialog {
  Q_OBJECT

public:
  explicit SecretInputDialog(QWidget *parent = 0);
  ~SecretInputDialog();

  std::string return_name;
  byte_seq return_value;

public slots:
  void accept() override;

private:
  std::unique_ptr<Ui::SecretInputDialog> ui;
};
