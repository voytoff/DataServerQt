#include "applicationsettingsloader.h"
#include "colorscheme.h"
#include "configurationselector.h"
#include "mainwindow.h"
#include "moduletypeinfo.h"
#include "paddeditemdelegate.h"
#include "servercontroller.h"
#include "loglevel.h"
#include "themedsvgicon.h"
#include "applicationsettingsdialog.h"
#include "applicationsettingsloader.h"
#include "settingssection.h"
#include "windowsettings.h"

#include <QCloseEvent>
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
#include <QDir>
#include <QApplication>
#include <QStyleHints>
#include <QMessageBox>

namespace qds
{

MainWindow::MainWindow(
  const QSqlDatabase& database,
  ServerController& controller,
  const QString &configFile,
  QWidget* parent)
  : QMainWindow(parent)
  , m_controller(controller)
  , m_configFile(configFile)
{
  setWindowTitle(
    "DataServer [" + database.databaseName() + "]");

  setWindowIcon(
    QIcon(":/images/server.png"));

  resize(1000, 650);

  createToolbar(database);
  createStatusbar(
    database.databaseName());

  m_splitter = new QSplitter(Qt::Vertical, this);
  m_splitter->setChildrenCollapsible(false);
  m_splitter->setHandleWidth(5);

  m_modulesTable = new QTableWidget(this);

  m_modulesTable->setFrameShape(QFrame::NoFrame);

  m_modulesTable->setItemDelegate(
    new PaddedItemDelegate(8, 0, 8, 0, this));

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

  m_splitter->addWidget(m_modulesTable);
  m_splitter->addWidget(m_logView);

  m_splitter->setStretchFactor(0, 1);
  m_splitter->setStretchFactor(1, 1);

  m_splitter->setSizes({300, 300});

  setCentralWidget(m_splitter);

  m_statisticsTimer = new QTimer(this);
  m_statisticsTimer->setInterval(1000);

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
      updateArchiveDirectory();
      updateControls();
    });

  connect(
    &m_controller,
    &ServerController::stopped,
    this,
    [this]()
    {
      updateServerState(false);
      updateControls();

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

  connect(
    &m_controller,
    &ServerController::moduleError,
    this,
    &MainWindow::setModuleError);


  if (m_controller.loadConfiguration())
    updateModules();
  else
    m_logView->appendPlainText(
      tr("ERROR: Не удалось загрузить конфигурацию"));

  updateServerState(
    m_controller.isRunning());

  updateArchiveDirectory();

  // восстановим положение и состояние окна
  restoreWindowSettings();
}

void MainWindow::createToolbar(
  const QSqlDatabase& database)
{
  auto* toolBar = addToolBar(tr("Управление"));
  toolBar->setIconSize(QSize(24, 24));

  toolBar->setMovable(false);
  toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

  m_startAction = toolBar->addAction(
    themedSvgIcon(":/images/start.svg"),
    tr("Запустить"));

  m_stopAction = toolBar->addAction(
    themedSvgIcon(":/images/stop.svg"),
    tr("Остановить"));

  toolBar->addSeparator();

  m_themeAction = toolBar->addAction(
    themedSvgIcon(":/images/theme.svg"),
    tr("Сменить тему"));

  m_themeAction->setToolTip(
    tr("Переключить светлую/тёмную тему"));

  toolBar->addSeparator();

  m_settingsAction = toolBar->addAction(
    themedSvgIcon(":/images/settings.svg"),
    tr("Настройки"));

  m_settingsAction->setToolTip(
    tr("Настройки приложения"));

  toolBar->addSeparator();

  auto* configurationLabel =
    new QLabel(tr("Эксперимент:"), this);

  toolBar->addWidget(configurationLabel);

  m_configurationSelector =
    new ConfigurationSelector(database, this);

  m_configurationSelector->setMinimumWidth(200);
  m_configurationSelector->setSizePolicy(
    QSizePolicy::Fixed,
    QSizePolicy::Preferred);

  toolBar->addWidget(m_configurationSelector);

  m_configurationSelector->setConfigurationId(
    m_controller.configurationId());

  toolBar->addSeparator();

  m_clearLogAction = toolBar->addAction(
    themedSvgIcon(":/images/clear.svg"),
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

  connect(
    m_configurationSelector,
    &ConfigurationSelector::configurationChanged,
    this,
    [this](ConfigurationId id)
    {
      if (!m_controller.setConfigurationId(id))
      {
        m_configurationSelector->setConfigurationId(
          m_controller.configurationId());

        m_logView->appendPlainText(
          tr("ERROR: Не удалось переключить эксперимент"));

        return;
      }

      updateModules();

      if (!ApplicationSettingsLoader::saveConfigurationId(
            m_configFile,
            id))
      {
        m_logView->appendPlainText(
          tr("ERROR: Не удалось сохранить выбранный эксперимент"));
      }
    });

  connect(
    m_themeAction,
    &QAction::triggered,
    this,
    &toggleColorScheme);

  connect(
    m_settingsAction,
    &QAction::triggered,
    this,
    &MainWindow::showSettings);
}

void MainWindow::createStatusbar(const QString& databaseName)
{
  auto* status = statusBar();

  m_statusLabel = new QLabel(tr("Остановлен"), this);

  status->addWidget(m_statusLabel);

  status->setSizeGripEnabled(true);

  m_archiveLabel = new QLabel(this);
  m_archiveLabel->setTextInteractionFlags(
    Qt::TextSelectableByMouse);

  status->addPermanentWidget(
    m_archiveLabel, 1);

  status->addPermanentWidget(
    new QLabel(
      databaseName,
      this));
}

void MainWindow::updateControls()
{
  const bool running =
    m_controller.isRunning();

  m_configurationSelector->setEnabled(
    !running);

  m_startAction->setEnabled(
    !running && m_controller.configurationLoaded());

  m_stopAction->setEnabled(
    running);

  m_settingsAction->setEnabled(
    !running);
}

void MainWindow::updateModules()
{  
  const auto& modules =
    m_controller.configuration().modules();

  m_modulesTable->clearContents();

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

void MainWindow::updateServerState(bool running)
{
  m_statusLabel->setText(
    running
      ? tr("Сервер работает")
      : tr("Сервер остановлен"));

  updateControls();
}

void MainWindow::updateModuleStatistics()
{
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

    const auto state =
      m_controller.state(id);

    if (state)
    {
      QString stateText;
      QColor background;

      switch (*state)
      {
      case DataStreamSourceState::Running:
        stateText = tr("Работает");
        break;

      case DataStreamSourceState::Stopped:
        stateText = tr("Остановлен");
        break;

      case DataStreamSourceState::Error:
        stateText = tr("Ошибка");
        background = QColor(255, 220, 220);
        break;
      }

      m_modulesTable->item(row, 2)->setText(
        stateText);

      for (int column = 0;
           column < m_modulesTable->columnCount();
           ++column)
      {
        if (auto* item =
            m_modulesTable->item(row, column))
        {
          item->setBackground(
            background.isValid()
              ? QBrush(background)
              : QBrush());
        }
      }
    }

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
        time.toString("HH:mm:ss"));
    }
  }
}

void MainWindow::updateArchiveDirectory()
{
  const auto& directory =
    m_controller.currentArchiveDirectory();

  const QString path =
    QDir::toNativeSeparators(
      QString::fromStdWString(
        directory.wstring()));

  m_archiveLabel->setText(path);
  m_archiveLabel->setToolTip(path);
}

void MainWindow::setModuleError(ModuleId id)
{
  for (int row = 0;
       row < m_modulesTable->rowCount();
       ++row)
  {
    auto* item = m_modulesTable->item(row, 0);

    if (!item ||
        item->data(Qt::UserRole).toUInt() != id.value)
      continue;

    for (int column = 0;
         column < m_modulesTable->columnCount();
         ++column)
    {
      if (auto* cell = m_modulesTable->item(row, column))
        cell->setBackground(QColor(255, 220, 220));
    }

    m_modulesTable->item(row, 2)->setText(
      tr("Ошибка"));

    break;
  }
}

void MainWindow::showSettings()
{
  if (m_controller.isRunning())
    return;

  const SettingsSections sections =
    SettingsSection::Database |
    SettingsSection::Archive |
    SettingsSection::Log;

  ApplicationSettings settings;
  QString error;

  if (!ApplicationSettingsLoader::load(
        m_configFile,
        settings,
        error,
        sections))
  {
    QMessageBox::warning(
      this,
      tr("Настройки приложения"),
      error);

    return;
  }

  ApplicationSettingsDialog dialog(
    settings,
    sections,
    this);

  connect(
    &dialog,
    &ApplicationSettingsDialog::settingsApplied,
    this,
    [this](SettingsSections changed)
    {
      if (changed == SettingsSections{})
        return;

      QMessageBox::information(
        this,
        tr("Настройки сохранены"),
        tr("Изменения сохранены в INI.\n"
          "Для применения новых настроек "
          "необходимо перезапустить приложение %1.")
          .arg(QCoreApplication::applicationName()));
    });

  dialog.exec();
}

void MainWindow::restoreWindowSettings()
{
  WindowSettings::restoreGeometry(
    m_configFile,
    "Window/Geometry",
    this);

  WindowSettings::restoreSplitter(
    m_configFile,
    "Window/MainSplitter",
    m_splitter);
}

void MainWindow::saveWindowSettings() const
{
  WindowSettings::saveGeometry(
    m_configFile,
    "Window/Geometry",
    this);

  WindowSettings::saveSplitter(
    m_configFile,
    "Window/MainSplitter",
    m_splitter);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  saveWindowSettings();

  QMainWindow::closeEvent(event);
}

}



