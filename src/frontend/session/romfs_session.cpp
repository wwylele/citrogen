#include "romfs_session.h"
#include "frontend/format_detect.h"
#include "frontend/util.h"
#include <QFileDialog>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <algorithm>

RomfsTreeItem::RomfsTreeItem(CB::ContainerPtr container_, std::size_t row_,
                             const QString &text_, RomfsTreeItem *parent_)
    : container(std::move(container_)), row(row_), text(text_),
      parent(parent_) {}

bool RomfsTreeItem::isExpandable() const { return !expanded; }

bool RomfsTreeItem::isDirectory() const {
  return !container->Value().has_value();
}

CB::ContainerPtr RomfsTreeItem::getContainer() { return container; }

void RomfsTreeItem::expand() {
  if (expanded)
    return;

  auto list = container->List();

  std::size_t row = 0;
  for (auto &name : list) {
    children.push_back(std::make_unique<RomfsTreeItem>(
        container->Open(name), row, QString::fromStdString(name), this));
    row++;
  }

  expanded = true;
}

const std::vector<std::unique_ptr<RomfsTreeItem>> &
RomfsTreeItem::getChildren() const {
  return children;
}

RomfsTreeModel::RomfsTreeModel(CB::ContainerPtr container, QObject *parent)
    : root(std::move(container)), QAbstractItemModel(parent) {}

QModelIndex RomfsTreeModel::index(int row, int column,
                                  const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent))
    return {};
  if (parent.isValid()) {
    RomfsTreeItem *parent_item =
        static_cast<RomfsTreeItem *>(parent.internalPointer());
    return createIndex(row, column, parent_item->getChildren()[row].get());
  }

  return createIndex(row, column, const_cast<RomfsTreeItem *>(&root));
}

QModelIndex RomfsTreeModel::parent(const QModelIndex &index) const {
  if (!index.isValid())
    return {};

  RomfsTreeItem *parent_item =
      static_cast<RomfsTreeItem *>(index.internalPointer())->parent;
  if (!parent_item) {
    return QModelIndex();
  }
  return createIndex(static_cast<int>(parent_item->row), 0, parent_item);
}

int RomfsTreeModel::rowCount(const QModelIndex &parent) const {
  if (!parent.isValid())
    return 1;

  RomfsTreeItem *parent_item =
      static_cast<RomfsTreeItem *>(parent.internalPointer());
  return static_cast<int>(parent_item->getChildren().size());
}

int RomfsTreeModel::columnCount(const QModelIndex &) const { return 1; }

QVariant RomfsTreeModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return {};

  switch (role) {
  case Qt::DisplayRole:
    return static_cast<RomfsTreeItem *>(index.internalPointer())->text;
  default:
    return {};
  }
}

bool RomfsTreeModel::hasChildren(const QModelIndex &parent) const {
  if (!parent.isValid())
    return true;

  RomfsTreeItem *parent_item =
      static_cast<RomfsTreeItem *>(parent.internalPointer());
  return parent_item->isDirectory();
}

bool RomfsTreeModel::canFetchMore(const QModelIndex &parent) const {
  if (!parent.isValid())
    return false;
  RomfsTreeItem *parent_item =
      static_cast<RomfsTreeItem *>(parent.internalPointer());
  return parent_item->isExpandable();
}
void RomfsTreeModel::fetchMore(const QModelIndex &parent) {
  if (!parent.isValid())
    return;
  RomfsTreeItem *parent_item =
      static_cast<RomfsTreeItem *>(parent.internalPointer());
  return parent_item->expand();
}

RomfsSession::RomfsSession(std::shared_ptr<Session> parent_session,
                           const QString &name, CB::ContainerPtr container_)
    : Session(parent_session, name, "RomFS (Read-only Filesystem)"),
      container(std::move(container_)) {
  tree = new QTreeView();
  RomfsTreeModel *model = new RomfsTreeModel(std::move(container), tree);
  tree->setModel(model);
  tree->setHeaderHidden(true);
  tree->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(tree, &QTreeView::customContextMenuRequested, this,
          &RomfsSession::onTreeContextMenu);

  QVBoxLayout *main_layout = new QVBoxLayout();
  main_layout->addWidget(tree);
  setupContentLayout(main_layout);
}

void RomfsSession::onTreeContextMenu(const QPoint &point) {
  QModelIndex index = tree->indexAt(point);
  if (index.isValid()) {
    auto item = static_cast<RomfsTreeItem *>(index.internalPointer());
    if (!item->isDirectory()) {
      QMenu menu;

      connect(menu.addAction(tr("Export...")), &QAction::triggered,
              [this, item]() {
                QString filename = QFileDialog::getSaveFileName(
                    this, tr("Exprt"), item->text, tr("All files (*.*)"));
                if (filename.isEmpty()) {
                  return;
                }
                QFile file(filename);
                if (!file.open(QFile::WriteOnly)) {
                  QMessageBox::critical(this, tr("Error"),
                                        tr("Failed to open the file!"));
                  return;
                }
                auto src = item->getContainer()->ValueT<FB::FilePtr>();

                auto buf = src->Read(0, src->GetSize());
                file.write((char *)buf.data(), buf.size());
                file.close();
              });

      connect(menu.addAction(tr("Open")), &QAction::triggered, [this, item]() {
        auto file = item->getContainer()->ValueT<FB::FilePtr>();
        auto session =
            TryCreateSession(file, item->text, this->shared_from_this());
        if (session) {
          openChildSession(item->text.toStdString() /*FIXME*/,
                           [session]() { return session; });
        }
      });

      menu.exec(tree->mapToGlobal(point));
    }
  }
}
