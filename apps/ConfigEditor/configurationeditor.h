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
class QComboBox;
class ConfigurationSelector;

namespace qds
{

class ConfigurationEditor final : public QWidget
{
  Q_OBJECT

public:
  explicit ConfigurationEditor(
    const QSqlDatabase& database,
    const QString& configFile,
    ConfigurationId configurationId,
    QWidget* parent = nullptr);

  void saveWindowSettings() const;

private:
  void restoreWindowSettings();

  void updateChannelVisibility(
    const QModelIndex& moduleIndex);

  ConfigurationId currentConfiguration() const;

  void createToolbar(
    const QSqlDatabase &database);

  void updateActions();

  void updateSensorTypes();

  void editCalibration();

  void editConfiguration();
  void addConfiguration();
  void removeConfiguration();

  void configurationChanged();

  void showSettings();

private:
  ConfigurationSelector* m_configurationCombo = nullptr;
  BaseTableView* m_tagsView = nullptr;
  QString m_configFile;

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
  QSplitter* m_splitterRight = nullptr;
  QSplitter* m_signalsSplitter = nullptr;


  QTabWidget* m_tabs = nullptr;

  QToolBar* m_toolBar = nullptr;

  QAction* m_addAction = nullptr;
  QAction* m_removeAction = nullptr;
  QAction* m_testAction = nullptr;
  QAction* m_calibrationAction = nullptr;
  QAction* m_configurationPropertiesAction = nullptr;
  QAction* m_configurationAddAction = nullptr;
  QAction* m_configurationRemoveAction = nullptr;
  QAction* m_themeAction = nullptr;
  QAction* m_settingsAction = nullptr;

signals:
  void statusMessage(
    const QString& message);
};

}