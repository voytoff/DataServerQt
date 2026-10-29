#pragma once

#include "datatypes.h"

#include <QMainWindow>
#include <QSqlDatabase>
#include <QAction>
#include <QLabel>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QTimer>
#include <QDesktopServices>
#include <QEvent>
#include <QMouseEvent>
#include <QUrl>

class QLabel;
class QPushButton;
class QTableWidget;
class QPlainTextEdit;
class ServerController;
class ConfigurationSelector;
class QSplitter;

namespace qds
{

class ServerController;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(
    const QSqlDatabase& database,
    ServerController& controller,
    const QString& configFile,
    QWidget* parent = nullptr);

protected:
  void closeEvent(
    QCloseEvent* event) override;

  bool eventFilter(
    QObject* watched,
    QEvent* event) override;

private:
  void createToolbar(
    const QSqlDatabase &database);

  void createStatusbar(
    const QString &databaseName);

  void updateControls();

  void updateModules();
  void updateServerState(bool running);
  void updateModuleStatistics();
  void updateArchiveDirectory();

  void setModuleError(
    ModuleId id);

  void showSettings();

  void restoreWindowSettings();
  void saveWindowSettings() const;

private:
  ServerController& m_controller;
  QString m_configFile;

  QLabel* m_statusLabel = nullptr;
  QLabel* m_archiveLabel = nullptr;

  QTableWidget* m_modulesTable = nullptr;
  QPlainTextEdit* m_logView = nullptr;
  ConfigurationSelector* m_configurationSelector = nullptr;

  QSplitter* m_splitter = nullptr;

  QAction* m_startAction = nullptr;
  QAction* m_stopAction = nullptr;
  QAction* m_clearLogAction = nullptr;
  QAction* m_themeAction = nullptr;
  QAction* m_settingsAction = nullptr;

  QPlainTextEdit* m_logEdit = nullptr;

  QTimer* m_statisticsTimer = nullptr;
};

}