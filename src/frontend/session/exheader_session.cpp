#include "exheader_session.h"
#include "frontend/util.h"
#include "rsa_session.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>

ExheaderSession::ExheaderSession(std::shared_ptr<Session> parent_session,
                                 const QString &name,
                                 CB::ContainerPtr container_)
    : Session(parent_session, name,
              "Exheader (SCI, ACI and NCCH signature public key)"),
      container(std::move(container_)) {
  QPushButton *signature_button =
      RsaSession::CreateButton(this, container->Open("Signature"));
  QHBoxLayout *layout_signature = new QHBoxLayout();
  layout_signature->addWidget(signature_button);
  layout_signature->addStretch(1);

  QTableWidget *table_sci = new QTableWidget();
  table_sci->verticalHeader()->hide();
  table_sci->setColumnCount(2);
  table_sci->setHorizontalHeaderLabels({tr("Attribute"), tr("Value")});

  auto sci_name = container->Open("Name")->ValueT<std::array<char, 8>>();
  table_sci->setRowCount(3);

  auto BoolToStr = [this](bool v) { return v ? tr("True") : tr("False"); };

  table_sci->setItem(0, 0, new QTableWidgetItem(tr("Name")));
  table_sci->setItem(0, 1,
                     new QTableWidgetItem(QString::fromStdString(
                         std::string(sci_name.begin(), sci_name.end()))));

  table_sci->setItem(1, 0, new QTableWidgetItem(tr("Compressed Code")));
  table_sci->setItem(1, 1,
                     new QTableWidgetItem(BoolToStr(
                         container->Open("IsCodeCompressed")->ValueT<bool>())));

  table_sci->setItem(2, 0, new QTableWidgetItem(tr("SD Application")));
  table_sci->setItem(2, 1,
                     new QTableWidgetItem(BoolToStr(
                         container->Open("IsSdApp")->ValueT<bool>())));

  QVBoxLayout *main_layout = new QVBoxLayout();
  main_layout->addLayout(layout_signature);
  main_layout->addWidget(table_sci);

  setupContentLayout(main_layout);
}