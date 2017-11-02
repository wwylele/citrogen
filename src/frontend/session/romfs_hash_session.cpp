#include "frontend/session/romfs_hash_session.h"
#include "frontend/util.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

RomfsHashVerifier::RomfsHashVerifier(CB::ContainerPtr container_)
    : container(std::move(container_)) {}

void RomfsHashVerifier::run() {
  CB::ContainerPtr levels[3] = {
      container->Open("Level0"),
      container->Open("Level1"),
      container->Open("Level2"),
  };

  u64 sizes[3] = {
      levels[0]->Open("Size")->ValueT<u64>(),
      levels[1]->Open("Size")->ValueT<u64>(),
      levels[2]->Open("Size")->ValueT<u64>(),
  };

  emit initProgress((int)(sizes[0] + sizes[1] + sizes[2]));

  int total = 0;
  int verified = 0;
  for (unsigned li = 0; li < 3; ++li) {
    emit appendLog(tr("Verifying level %1 over level %2").arg(li).arg(li + 1));
    for (u64 i = 0; i < sizes[li]; ++i) {
      if (isInterruptionRequested()) {
        emit appendLog(tr("Canceled"));
        return;
      }

      if (levels[li]
              ->Open(CB::WithIndex("Hash", i))
              ->Open("Match")
              ->ValueT<bool>()) {
        ++verified;
      } else {
        emit appendLog(tr("Hash %1 mismatch").arg(i));
      }

      ++total;
      emit updateProgress(total);
    }
  }

  emit appendLog(
      tr("Finished with %1 passed / %2 total").arg(verified).arg(total));
}

RomfsHashSession::RomfsHashSession(std::shared_ptr<Session> parent_session,
                                   const QString &name,
                                   CB::ContainerPtr container_)
    : Session(parent_session, name, "RomFS IVFC Hash Tree"),
      container(std::move(container_)) {
  button_start = new QPushButton(tr("Start Verification"));
  connect(button_start, &QPushButton::clicked, this,
          &RomfsHashSession::onStartButton);
  QHBoxLayout *layout_button = new QHBoxLayout();
  layout_button->addWidget(button_start);
  layout_button->addStretch(1);

  progress_bar = new QProgressBar();

  edit_log = new QPlainTextEdit();

  QVBoxLayout *main_layout = new QVBoxLayout();
  main_layout->addLayout(layout_button);
  main_layout->addWidget(progress_bar);
  main_layout->addWidget(edit_log);

  setupContentLayout(main_layout);
}

RomfsHashSession::~RomfsHashSession() {
  if (thread != nullptr) {
    thread->requestInterruption();
    thread->wait();
    thread->deleteLater();
  }
}

void RomfsHashSession::onInitProgress(int count) {
  progress_bar->setRange(0, count);
  progress_bar->reset();
  edit_log->clear();
}

void RomfsHashSession::onUpdateProgress(int count) {
  progress_bar->setValue(count);
}

void RomfsHashSession::onAppendLog(const QString &str) {
  edit_log->appendPlainText(str);
}

void RomfsHashSession::onFinished() {
  thread->deleteLater();
  thread = nullptr;
  button_start->setText(tr("Start Verification"));
}

void RomfsHashSession::onStartButton() {
  if (thread != nullptr) {
    thread->requestInterruption();
    thread->wait();
    return;
  }

  button_start->setText(tr("Cancel Verification"));
  thread = new RomfsHashVerifier(container);
  connect(thread, &RomfsHashVerifier::initProgress, this,
          &RomfsHashSession::onInitProgress);
  connect(thread, &RomfsHashVerifier::updateProgress, this,
          &RomfsHashSession::onUpdateProgress);
  connect(thread, &RomfsHashVerifier::appendLog, this,
          &RomfsHashSession::onAppendLog);
  connect(thread, &RomfsHashVerifier::finished, this,
          &RomfsHashSession::onFinished);
  thread->start();
}