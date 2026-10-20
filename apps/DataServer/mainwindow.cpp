#include "mainwindow.h"
#include "moduletypeinfo.h"
#include "servercontroller.h"
#include "loglevel.h"

#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QDateTime>

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

  createToolbar();

  auto* splitter = new QSplitter(Qt::Vertical, this);
  splitter->setChildrenCollapsible(false);
  splitter->setHandleWidth(5);

  m_statusLabel = new QLabel(tr("Остановлен"), this);

  auto* status = statusBar();

  status->addWidget(m_statusLabel);

  status->addPermanentWidget(
    new QLabel(
      database.databaseName(),
      this));

  status->setSizeGripEnabled(true);

  m_modulesTable = new QTableWidget(this);
  m_modulesTable->setFrameShape(QFrame::NoFrame);

  m_modulesTable->setStyleSheet(
    "QTableWidget::item {padding: 0 8px 0 8px;}");

  m_modulesTable->setColumnCount(6);
  m_modulesTable->setHorizontalHeaderLabels({
    "Модуль",
    "Тип",
    "Состояние",
    "Блоков",
    "Кадров",
    "Последние данные"
  });

  m_modulesTable->setSelectionBehavior(
    QAbstractItemView::SelectRows);

  m_modulesTable->setSelectionMode(
    QAbstractItemView::SingleSelection);

  m_modulesTable->setEditTriggers(
    QAbstractItemView::NoEditTriggers);

  auto* vHeader = m_modulesTable->verticalHeader();
  vHeader->setVisible(false);
  vHeader->setDefaultSectionSize(26);

  auto* header = m_modulesTable->horizontalHeader();

  header->setSectionResizeMode(
    0, QHeaderView::Stretch);

  for (int column = 1; column < 6; ++column)
  {
    header->setSectionResizeMode(
      column, QHeaderView::ResizeToContents);
  }

  m_modulesTable->setShowGrid(false);

  m_logView = new QPlainTextEdit(this);
  m_logView->setReadOnly(true);
  m_logView->setMaximumBlockCount(10000);
  m_logView->setFrameShape(QFrame::NoFrame);
  m_logView->setLineWrapMode(QPlainTextEdit::NoWrap);

  splitter->addWidget(m_modulesTable);
  splitter->addWidget(m_logView);

  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 1);

  splitter->setSizes({300, 300});

  setCentralWidget(splitter);

  m_statisticsTimer = new QTimer(this);
  m_statisticsTimer->setInterval(500);

  connect(
    m_statisticsTimer,
    &QTimer::timeout,
    this,
    &MainWindow::updateModuleStatistics);

  m_statisticsTimer->start();

  connect(
    &m_controller,
    &ServerController::started,
    this,
    [this]()
    {
      updateServerState(true);

      for (int row = 0;
           row < m_modulesTable->rowCount();
           ++row)
      {
        if (auto* item = m_modulesTable->item(row, 2))
          item->setText(tr("Работает"));
      }
    });

  connect(
    &m_controller,
    &ServerController::stopped,
    this,
    [this]()
    {
      updateServerState(false);

      for (int row = 0;
           row < m_modulesTable->rowCount();
           ++row)
      {
        m_modulesTable->item(row, 2)
        ->setText(tr("Остановлен"));
      }
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

  if (m_controller.loadConfiguration())
  {
    updateModules();
  }
  else
  {
    m_logView->appendPlainText(
      tr("ERROR: Не удалось загрузить конфигурацию"));
  }

  updateServerState(
    m_controller.isRunning());
}

void MainWindow::createToolbar()
{
  auto* toolBar = addToolBar(tr("Управление"));
  toolBar->setIconSize(QSize(16, 16));

  toolBar->setMovable(false);
  toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

  m_startAction = toolBar->addAction(
    QIcon(":/images/start.png"),
    tr("Запустить"));

  m_stopAction = toolBar->addAction(
    QIcon(":/images/stop.png"),
    tr("Остановить"));

  toolBar->addSeparator();

  m_clearLogAction = toolBar->addAction(
    QIcon(":/images/clear.png"),
    tr("Очистить журнал"));

  connect(
    m_startAction,
    &QAction::triggered,
    this,
    [this]()
    {
      if (!m_controller.start())
        m_logView->appendPlainText(
          tr("ERROR: Не удалось запустить сервер"));
    });

  connect(
    m_stopAction,
    &QAction::triggered,
    this,
    [this]()
    {
      m_controller.stop();
    });

  connect(
    m_clearLogAction,
    &QAction::triggered,
    this,
    [this]()
    {
      m_logView->clear();
    });
}

void MainWindow::updateModules()
{
  const auto& modules =
    m_controller.configuration().modules();

  m_modulesTable->setRowCount(
    static_cast<int>(modules.size()));

  int row = 0;

  for (const auto& module : modules)
  {
    auto* nameItem =
      new QTableWidgetItem(module.serial);

    auto* typeItem =
      new QTableWidgetItem(
        moduleTypeToString(module.type));

    auto* stateItem =
      new QTableWidgetItem(tr("Остановлен"));

    nameItem->setData(
      Qt::UserRole,
      QVariant::fromValue(module.id.value));

    m_modulesTable->setItem(row, 0, nameItem);
    m_modulesTable->setItem(row, 1, typeItem);
    m_modulesTable->setItem(row, 2, stateItem);

    for (int column = 3; column < 6; ++column)
    {
      auto* item = new QTableWidgetItem("—");

      if (column == 3 || column == 4)
        item->setTextAlignment(
          Qt::AlignRight | Qt::AlignVCenter);

      m_modulesTable->setItem(row, column, item);
    }

    ++row;
  }
}

void MainWindow::updateServerState(
  bool running)
{
  m_startAction->setEnabled(!running);
  m_stopAction->setEnabled(running);

  m_statusLabel->setText(
    running
      ? tr("Сервер работает")
      : tr("Сервер остановлен"));
}

void MainWindow::updateModuleStatistics()
{
  if (!m_controller.isRunning())
    return;

  for (int row = 0;
       row < m_modulesTable->rowCount();
       ++row)
  {
    const auto* moduleItem =
      m_modulesTable->item(row, 0);

    if (!moduleItem)
      continue;

    const ModuleId id{
      moduleItem->data(Qt::UserRole).toUInt()
    };

    const auto statistics =
      m_controller.statistics(id);

    if (!statistics)
      continue;

    const auto& stats = *statistics;

    m_modulesTable->item(row, 3)->setText(
      QString::number(stats.blocksReceived));

    m_modulesTable->item(row, 4)->setText(
      QString::number(stats.framesReceived));

    if (stats.lastTimestamp != 0)
    {
      const auto time =
        QDateTime::fromMSecsSinceEpoch(
          static_cast<qint64>(
            stats.lastTimestamp / 1000));

      m_modulesTable->item(row, 5)->setText(
        time.toString("HH:mm:ss.zzz"));
    }
  }
}

}