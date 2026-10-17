#pragma once

#include "basecomboboxdelegate.h"
#include "configurationpropertiesmodel.h"
#include "configurationsignalmodel.h"
#include "delegate/configurationsignalpropertiesdelegate.h"
#include "configurationsignalpropertiesmodel.h"
#include "configurationtreemodel.h"
#include "basetableview.h"
#include "db/configurationrepository.h"
#include "sensormodel.h"
#include "sensortypemodel.h"
#include "sensorpropertiesmodel.h"

#include <QWidget>

class QSqlTableModel;
class QSqlRelationalTableModel;
class QTableView;
class QTreeView;
class QSqlDatabase;
class QToolBar;
class QAction;
class QSplitter;

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

  void createToolbar();
  void updateActions();

  void updateSensorTypes();

  void editCalibration();

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

  SensorTypeModel* m_sensorTypeModel = nullptr;
  BaseTableView* m_sensorTypesView = nullptr;
  BaseComboBoxDelegate* m_sensorTypeDelegate = nullptr;

  SensorModel* m_sensorModel = nullptr;
  BaseTableView* m_sensorsView = nullptr;

  SensorPropertiesModel* m_sensorPropertiesModel = nullptr;
  BaseTableView* m_sensorPropertiesView = nullptr;
  QSplitter* m_sensorsSplitter = nullptr;

  QTabWidget* m_tabs = nullptr;

  QToolBar* m_toolBar = nullptr;

  QAction* m_addAction = nullptr;
  QAction* m_removeAction = nullptr;
  QAction* m_testAction = nullptr;
  QAction* m_calibrationAction = nullptr;

signals:
  void statusMessage(
    const QString& message);
};

}