#pragma once

#include "core/container.h"
#include "frontend/session.h"
#include <QAbstractItemModel>

class RomfsTreeItem : public QObject {
  Q_OBJECT

public:
  RomfsTreeItem(CB::ContainerPtr container_, std::size_t row_ = 0,
                const QString &text_ = tr("(root)"),
                RomfsTreeItem *parent_ = nullptr);
  bool isExpandable() const;
  void expand();
  const std::vector<std::unique_ptr<RomfsTreeItem>> &getChildren() const;
  bool isDirectory() const;
  CB::ContainerPtr getContainer();
  const std::size_t row;
  const QString text;
  RomfsTreeItem *const parent;

private:
  CB::ContainerPtr container;
  bool expanded = false;
  std::vector<std::unique_ptr<RomfsTreeItem>> children;
};

class RomfsTreeModel : public QAbstractItemModel {
  Q_OBJECT

public:
  RomfsTreeModel(CB::ContainerPtr container, QObject *parent = nullptr);

  QVariant data(const QModelIndex &index, int role) const override;
  QModelIndex index(int row, int column,
                    const QModelIndex &parent) const override;
  QModelIndex parent(const QModelIndex &index) const override;
  bool hasChildren(const QModelIndex &parent) const;
  int rowCount(const QModelIndex &parent) const override;
  int columnCount(const QModelIndex &parent) const override;

protected:
  bool canFetchMore(const QModelIndex &parent) const override;
  void fetchMore(const QModelIndex &parent) override;

private:
  RomfsTreeItem root;
};

class QTreeView;

class RomfsSession : public Session {
  Q_OBJECT
public:
  RomfsSession(std::shared_ptr<Session> parent_session, const QString &name,
               CB::ContainerPtr container_);

private slots:
  void onTreeContextMenu(const QPoint &point);

private:
  QTreeView *tree;
  CB::ContainerPtr container;
};