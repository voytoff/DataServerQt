#pragma once

#include <QMainWindow>
#include <QSqlDatabase>
#include <QAction>
#include <QLabel>
#include <QPlainTextEdit>
#include <QTableWidget>

class QLabel;
class QPushButton;
class QTableWidget;
class QPlainTextEdit;

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
    QWidget* parent = nullptr);

private:
  void updateModules();
  void updateServerState(bool running);

private:
  ServerController& m_controller;

  QLabel* m_statusLabel = nullptr;

  QTableWidget* m_modulesTable = nullptr;
  QPlainTextEdit* m_logView = nullptr;

  QAction* m_startAction = nullptr;
  QAction* m_stopAction = nullptr;
  QAction* m_clearLogAction = nullptr;

  QPlainTextEdit* m_logEdit = nullptr;

};

}