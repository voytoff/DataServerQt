#pragma once

#include <QMainWindow>
#include <QSqlDatabase>

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
  ServerController& m_controller;

  QLabel* m_statusLabel = nullptr;

  QPushButton* m_startButton = nullptr;
  QPushButton* m_stopButton = nullptr;

  QTableWidget* m_modulesTable = nullptr;
  QPlainTextEdit* m_logView = nullptr;

};

}