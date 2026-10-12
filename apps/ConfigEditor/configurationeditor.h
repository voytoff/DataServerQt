#pragma once

#include "configurationpropertiesmodel.h"
#include "configurationsignalmodel.h"
#include "configurationsignalpropertiesdelegate.h"
#include "configurationsignalpropertiesmodel.h"
#include "configurationtreemodel.h"
#include "basetableview.h"
#include "db/configurationrepository.h"

#include <QWidget>

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
  BaseTableView* m_configurationsView = nullptr;
  BaseTableView* m_tagsView = nullptr;

  QSqlTableModel* m_configurations = nullptr;
  QSqlRelationalTableModel* m_tags = nullptr;

  ConfigurationRepository m_repository;
  ConfigurationTreeModel* m_treeModel = nullptr;
  QTreeView* m_configurationTree = nullptr;

  ConfigurationPropertiesModel* m_propertiesModel = nullptr;

  ConfigurationSignalModel* m_signalModel = nullptr;
  BaseTableView* m_signalsView = nullptr;

  ConfigurationSignalPropertiesModel* m_signalPropertiesModel = nullptr;

  BaseTableView* m_signalPropertiesView = nullptr;

  ConfigurationSignalPropertiesDelegate* m_signalPropertiesDelegate = nullptr;

signals:
  void statusMessage(
    const QString& message);
};

}