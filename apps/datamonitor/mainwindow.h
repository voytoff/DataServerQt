#pragma once

#include "datatypes.h"
#include "signalselectionmodel.h"
#include "clientconnection.h"

#include <QHostAddress>
#include <QMainWindow>
#include <QSqlDatabase>
#include <QString>

namespace qds
{

class BaseTableView;
class ConfigurationSelector;

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
};

}