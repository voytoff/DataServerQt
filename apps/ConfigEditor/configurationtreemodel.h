#pragma once

#include "db/configurationrepository.h"
#include "treeitem.h"
#include <QObject>
#include <QAbstractItemModel>

namespace qds
{

class ConfigurationTreeModel final
  : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit ConfigurationTreeModel(
    ConfigurationRepository& repository,
    QObject* parent = nullptr);

  [[nodiscard]]
  bool load(
    ConfigurationId configuration);

  QModelIndex index(
    int row,
    int column,
    const QModelIndex& parent = {}) const override;

  QModelIndex parent(
    const QModelIndex& index) const override;

  int rowCount(
    const QModelIndex& parent = {}) const override;

  int columnCount(
    const QModelIndex& parent = {}) const override;

  QVariant data(
    const QModelIndex& index,
    int role = Qt::DisplayRole) const override;

  bool setData(
    const QModelIndex& index,
    const QVariant& value,
    int role = Qt::EditRole) override;

  Qt::ItemFlags flags(
    const QModelIndex& index) const override;

private:
  ConfigurationRepository& m_repository;

  ConfigurationId m_configuration;

  std::unique_ptr<TreeItem> m_root;
};

}
