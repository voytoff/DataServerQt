#pragma once

#include "basecomboboxdelegate.h"
#include "configurationpropertiesmodel.h"
#include "configurationsignalmodel.h"
#include "delegate/configurationsignalpropertiesdelegate.h"
#include "configurationsignalpropertiesmodel.h"
#include "configurationtreemodel.h"
#include "basetableview.h"
#include "db/configurationrepository.h"
#include "mainwindow.h"
#include "sensormodel.h"
#include "sensortypemodel.h"
#include "sensorpropertiesmodel.h"

#include <QWidget>
#include <QSortFilterProxyModel>

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

/*
CREATE TABLE measurement_unit (
  id INT NOT NULL AUTO_INCREMENT,
  name VARCHAR(128) NOT NULL,
  symbol VARCHAR(32) NOT NULL,
  PRIMARY KEY (id),
  UNIQUE KEY uq_measurement_unit_name (name),
  UNIQUE KEY uq_measurement_unit_symbol (symbol)
)
ENGINE=InnoDB
DEFAULT CHARSET=utf8mb4
COLLATE=utf8mb4_unicode_ci;

INSERT INTO measurement_unit (name, symbol)
VALUES
  ('Безразмерная величина', '1'),
  ('Вольт', 'В'),
  ('Милливольт', 'мВ'),
  ('Ампер', 'А'),
  ('Миллиампер', 'мА'),
  ('Градус Цельсия', '°C'),
  ('Паскаль', 'Па'),
  ('Килопаскаль', 'кПа'),
  ('Мегапаскаль', 'МПа'),
  ('Герц', 'Гц'),
  ('Метр', 'м'),
  ('Миллиметр', 'мм'),
  ('Секунда', 'с');


  ALTER TABLE signal_type
  ADD COLUMN measurement_unit_id INT DEFAULT NULL
    AFTER description,
  ADD KEY ix_signal_type_measurement_unit
    (measurement_unit_id),
  ADD CONSTRAINT fk_signal_type_measurement_unit
    FOREIGN KEY (measurement_unit_id)
    REFERENCES measurement_unit (id)
    ON UPDATE CASCADE
    ON DELETE RESTRICT;
 */

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

  void saveWindowSettings(
    const MainWindow *window) const;

  void restoreWindowSettings(
    MainWindow *window);

private:
  void updateChannelVisibility(
    const QModelIndex& moduleIndex);

  ConfigurationId currentConfiguration() const;

  void createToolbar(
    const QSqlDatabase &database);

  void updateActions();

  void updateSensorTypes();
  void updateSensors();

  void editCalibration();

  void editConfiguration();
  void addConfiguration();
  void removeConfiguration();

  void configurationChanged();

  void showSettings();

  [[nodiscard]]
  BaseTableView* currentSignalsView() const noexcept;

  [[nodiscard]]
  QModelIndex currentSignalIndex() const;

  void validateConfiguration();

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
  QSplitter* m_modulesSplitter = nullptr;
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

  // разделение моделей
  QTabWidget* m_signalTabs = nullptr;

  BaseTableView* m_rawSignalsView = nullptr;
  BaseTableView* m_calculatedSignalsView = nullptr;

  QSortFilterProxyModel* m_rawSignalsProxy = nullptr;
  QSortFilterProxyModel* m_calculatedSignalsProxy = nullptr;

signals:
  void statusMessage(
    const QString& message);
};

}