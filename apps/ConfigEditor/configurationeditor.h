#pragma once

#include "configurationpropertiesmodel.h"
#include "configurationtreemodel.h"
#include "db/configurationrepository.h"
#include <QWidget>
#include <qtableview.h>
#include <qtreeview.h>

class QSqlTableModel;
class QSqlRelationalTableModel;
class QTableView;
class QTreeView;
class QSqlDatabase;

namespace qds
{

class ConfigurationEditor final : public QWidget
{
  Q_OBJECT

public:
  explicit ConfigurationEditor(
    const QSqlDatabase& database,
    QWidget* parent = nullptr);

private:
  void updateChannelVisibility(
    const QModelIndex& moduleIndex);

private:
  QTableView* m_configurationsView = nullptr;
  //QTableView* m_modulesView = nullptr;
  QTableView* m_tagsView = nullptr;

  QSqlTableModel* m_configurations = nullptr;
  //QSqlRelationalTableModel* m_modules = nullptr;
  QSqlRelationalTableModel* m_tags = nullptr;

  ConfigurationRepository m_repository;
  ConfigurationTreeModel* m_treeModel = nullptr;
  QTreeView* m_configurationTree = nullptr;

  ConfigurationPropertiesModel* m_propertiesModel = nullptr;
};

}