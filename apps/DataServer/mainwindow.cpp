#include "mainwindow.h"
#include "servercontroller.h"
#include "loglevel.h"

#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

namespace qds
{

MainWindow::MainWindow(
  const QSqlDatabase& database,
  ServerController& controller,
  QWidget* parent)
  : QMainWindow(parent)
  , m_controller(controller)
{
  setWindowTitle(
    "DataServer [" + database.databaseName() + "]");

  setWindowIcon(
    QIcon(":/images/server.png"));

  resize(1000, 650);

  auto* central = new QWidget(this);
  auto* layout = new QVBoxLayout(central);

  auto* controls = new QHBoxLayout;

  m_statusLabel = new QLabel("Остановлен", this);

  m_startButton = new QPushButton("Запустить", this);
  m_stopButton = new QPushButton("Остановить", this);

  m_stopButton->setEnabled(false);

  controls->addWidget(m_statusLabel);
  controls->addStretch();
  controls->addWidget(m_startButton);
  controls->addWidget(m_stopButton);

  layout->addLayout(controls);

  m_modulesTable = new QTableWidget(this);

  m_modulesTable->setColumnCount(3);
  m_modulesTable->setHorizontalHeaderLabels(
    {"Модуль", "Тип", "Состояние"});

  m_modulesTable->horizontalHeader()
    ->setStretchLastSection(true);

  layout->addWidget(m_modulesTable);

  m_logView = new QPlainTextEdit(this);
  m_logView->setReadOnly(true);
  m_logView->setMaximumBlockCount(10000);

  layout->addWidget(m_logView);

  setCentralWidget(central);

  connect(
    m_startButton,
    &QPushButton::clicked,
    this,
    [this]()
    {
      if (!m_controller.start())
        m_logView->appendPlainText(
          "ERROR: Не удалось запустить сервер");
    });

  connect(
    m_stopButton,
    &QPushButton::clicked,
    this,
    [this]()
    {
      m_controller.stop();
    });

  connect(
    &m_controller,
    &ServerController::started,
    this,
    [this]()
    {
      m_statusLabel->setText("Работает");
      m_startButton->setEnabled(false);
      m_stopButton->setEnabled(true);
    });

  connect(
    &m_controller,
    &ServerController::stopped,
    this,
    [this]()
    {
      m_statusLabel->setText("Остановлен");
      m_startButton->setEnabled(true);
      m_stopButton->setEnabled(false);
    });

  connect(
    &m_controller,
    &ServerController::logMessage,
    this,
    [this](LogLevel level, const QString& message)
    {
      m_logView->appendPlainText(
        QString::fromStdString(logLevelToString(level))
        + ": " + message);
    },
    Qt::QueuedConnection);
}

}