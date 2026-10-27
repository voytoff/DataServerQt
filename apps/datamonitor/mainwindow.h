#pragma once

#include "datatypes.h"
#include "signalselectionmodel.h"
#include "clientconnection.h"

#include <QHostAddress>
#include <QMainWindow>
#include <QSqlDatabase>
#include <QString>
#include <QChart>
#include <QChartView>
#include <QSplitter>

class QSplitter;
class QComboBox;

namespace qds
{

class BaseTableView;
class ConfigurationSelector;
class SignalChartWidget;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(
    const QSqlDatabase& database,
    const QString& configFile,
    ConfigurationId configurationId,
    QWidget* parent = nullptr);

private:
  void createToolbar();
  void createClient();

  bool loadConfiguration(
    ConfigurationId id);

  void connectToServer();
  void updateSubscription();

  bool openConnection();

  void startMonitoring();
  void stopMonitoring();
  void updateControls();
  void updateChartSignals();

  enum class MonitorState
  {
    Stopped,
    Subscribing,
    Running,
    Unsubscribing
  };

  void setMonitorState(MonitorState state);

private:
  ClientConnection* m_client = nullptr;

  QHostAddress m_serverAddress;
  quint16 m_serverPort = 0;

  QSqlDatabase m_database;
  QString m_configFile;
  ConfigurationId m_configurationId;

  ConfigurationSelector* m_configurationSelector = nullptr;
  SignalSelectionModel* m_signalModel = nullptr;
  BaseTableView* m_signalView = nullptr;

  QToolBar* m_toolBar = nullptr;

  QAction* m_monitorAction = nullptr;

  MonitorState m_monitorState = MonitorState::Stopped;

  QSplitter* m_splitter = nullptr;
  SignalChartWidget* m_signalChart = nullptr;

  QComboBox* m_chartIntervalCombo = nullptr;
  QComboBox* m_yAxisModeCombo = nullptr;

};

}