#pragma once

#include "core/container.h"
#include "frontend/session.h"
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QThread>

class RomfsHashVerifier : public QThread {
  Q_OBJECT
public:
  RomfsHashVerifier(CB::ContainerPtr container_);

protected:
  void run() override;

signals:
  void initProgress(int count);
  void updateProgress(int count);
  void appendLog(const QString &str);

private:
  CB::ContainerPtr container;
};

class RomfsHashSession : public Session {
  Q_OBJECT
public:
  RomfsHashSession(std::shared_ptr<Session> parent_session, const QString &name,
                   CB::ContainerPtr container_);
  ~RomfsHashSession();
public slots:
  void onInitProgress(int count);
  void onUpdateProgress(int count);
  void onAppendLog(const QString &str);
  void onFinished();

private slots:
  void onStartButton();

private:
  CB::ContainerPtr container;
  QPushButton *button_start;
  QProgressBar *progress_bar;
  QPlainTextEdit *edit_log;
  RomfsHashVerifier *thread = nullptr;
};