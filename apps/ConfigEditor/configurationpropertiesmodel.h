#pragma once

#include "configurationtreemodel.h"

#include <QAbstractTableModel>
#include <QJsonObject>
#include <QModelIndex>
#include <QPersistentModelIndex>

#include <vector>

namespace qds
{

class ConfigurationPropertiesModel final
  : public QAbstractTableModel
{
public:
  explicit ConfigurationPropertiesModel(
    ConfigurationTreeModel* treeModel,
    QObject* parent = nullptr);

  void setItem(
    const QModelIndex& index);

  int rowCount(
    const QModelIndex& parent = {}) const override;

  int columnCount(
    const QModelIndex& parent = {}) const override;

  QVariant data(
    const QModelIndex& index,
    int role = Qt::DisplayRole) const override;

  QVariant headerData(
    int section,
    Qt::Orientation orientation,
    int role = Qt::DisplayRole) const override;

  bool setData(
    const QModelIndex& index,
    const QVariant& value,
    int role = Qt::EditRole) override;

  Qt::ItemFlags flags(
    const QModelIndex& index) const override;

private:
  ConfigurationTreeModel* m_treeModel = nullptr;

  struct Property
  {
    QString name;
    QVariant value;
  };

  std::vector<Property> m_properties;

  QPersistentModelIndex m_currentItem;

};

}