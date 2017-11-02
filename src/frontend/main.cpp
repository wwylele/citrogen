
#include "frontend/main.h"
#include "core/file_backend/disk_file.h"
#include "core/secret_backend/secret_database.h"
#include "frontend/format_detect.h"
#include "frontend/tab_widget.h"
#include "secret/secret_config.h"
#include <QMenu>
#include <QMessageBox>
#include <QStandardPaths>
#include <QtGui>
#include <QtWidgets>

MainWindow::MainWindow() {
  TabWidget *tab_widget = new TabWidget();
  connect(this, &MainWindow::openNewSession, tab_widget,
          &TabWidget::addSession);
  setCentralWidget(tab_widget);

  auto menu_file = menuBar()->addMenu(tr("&File"));
  connect(menu_file->addAction(tr("&Open...")), &QAction::triggered, this,
          &MainWindow::onOpen);

  connect(menu_file->addAction(tr("Manage Secrets...")), &QAction::triggered,
          this, &MainWindow::onManageSecrets);

  setWindowTitle(tr("Citrogen"));
  setMinimumSize(160, 160);
  resize(1024, 768);

  Session::InitResource();
}

void MainWindow::onManageSecrets() {
  auto secrets = SB::Lock();
  if (SecretConfigDialog(secrets, this).exec() == QDialog::Rejected) {
    SB::Discard(std::move(secrets));
    return;
  }
  SB::Unlock(std::move(secrets));
}

void MainWindow::onOpen() {
  QString filename = QFileDialog::getOpenFileName(this, tr("Open"), QString(),
                                                  tr("All files (*.*)"));
  if (filename.isEmpty()) {
    return;
  }
  QFileInfo file_info(filename);
  auto file = FB::OpenDiskFile(filename.toStdString());

  if (!file) {
    QMessageBox::critical(this, tr("Error"), tr("Failed to open the file!"));
    return;
  }

  auto session = TryCreateSession(file, file_info.fileName());

  if (session) {
    emit openNewSession(session);
    return;
  }

  QMessageBox::critical(this, tr("Error"),
                        tr("Failed to detect the file format!"));
}

#ifdef main
#undef main
#endif

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QCoreApplication::setApplicationName("Citrogen");

  QDir appdata =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  appdata.mkpath(".");
  SB::Init(QFileInfo(appdata, "secret").absoluteFilePath().toStdString());

  MainWindow main_window;

  main_window.show();
  return app.exec();
}