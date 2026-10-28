#include "mainwindow.h"

#include "applicationsettingsloader.h"
#include "basetableview.h"
#include "configurationrepository.h"
#include "configurationselector.h"
#include "monitorsettingsloader.h"
#include "signalchartwidget.h"
#include "themedsvgicon.h"
#include "colorscheme.h"

#include <QSignalBlocker>
#include <QHeaderView>
#include <QMessageBox>
#include <QSizePolicy>
#include <QToolBar>
#include <QLabel>
#include <QSettings>
#include <QStatusBar>
#include <QDebug>
#include <QSplitter>
#include <QApplication>
#include <QStyleHints>

namespace qds
{

MainWindow::MainWindow(
  const QSqlDatabase& database,
  const QString& configFile,
  ConfigurationId configurationId,
  QWidget* parent)
  : QMainWindow(parent)
  , m_database(database)
  , m_configFile(configFile)
  , m_configurationId(configurationId)
{
  setWindowTitle("DataMonitor");

  setWindowIcon(
    QIcon(":/images/main.png"));

  statusBar()->setSizeGripEnabled(true);

  resize(1100, 700);

  createToolbar();

  // Таблица сигналов

  m_signalModel =
    new SignalSelectionModel(this);

  m_signalView =
    new BaseTableView(0, 4, 0, 4, 0, this);

  m_signalView->setModel(m_signalModel);

  auto* header =
    m_signalView->horizontalHeader();

  header->setSectionResizeMode(
    0, QHeaderView::Fixed);

  header->resizeSection(0, 70);

  header->setSectionResizeMode(
    1, QHeaderView::Fixed);

  header->resizeSection(1, 100);

  header->setSectionResizeMode(
    2, QHeaderView::Stretch);

  header->setSectionResizeMode(
    3, QHeaderView::Fixed);

  header->resizeSection(3, 150);


  // Таблица и график.
  m_splitter = new QSplitter(
    Qt::Vertical,
    this);

  m_splitter->setChildrenCollapsible(false);
  m_splitter->setHandleWidth(5);

  m_splitter->addWidget(m_signalView);

  m_signalChart =
    new SignalChartWidget(m_splitter);

  m_splitter->addWidget(m_signalChart);

  // Начальные размеры панелей.
  m_splitter->setSizes({
    250,
    400
  });

  m_splitter->setChildrenCollapsible(false);

  setCentralWidget(m_splitter);


  // Создаём клиент до открытия соединения.
  createClient();

  // Загружаем начальную конфигурацию
  if (!loadConfiguration(m_configurationId))
  {
    QMessageBox::critical(
      this,
      tr("DataMonitor"),
      tr("Не удалось загрузить конфигурацию."));
  }
  openConnection();

  // Устанавливаем выбранный эксперимент
  // до подключения сигнала.

  m_configurationSelector->setConfigurationId(
    m_configurationId);

  // Переключение эксперимента

  connect(
    m_configurationSelector,
    &ConfigurationSelector::configurationChanged,
    this,
    [this](ConfigurationId id)
    {
      if (id == m_configurationId)
        return;

      const quint16 previousPort = m_serverPort;

      if (!loadConfiguration(id))
      {
        QSignalBlocker blocker(m_configurationSelector);

        m_configurationSelector->setConfigurationId(
          m_configurationId);

        QMessageBox::warning(
          this,
          tr("DataMonitor"),
          tr("Не удалось переключить эксперимент."));

        return;
      }

      if (m_serverPort != previousPort ||
          !m_client->isOpen())
      {
        openConnection();
      }

      if (!ApplicationSettingsLoader::saveConfigurationId(
            m_configFile,
            id))
      {
        QMessageBox::warning(
          this,
          tr("DataMonitor"),
          tr("Не удалось сохранить выбранный эксперимент."));
      }
    });

  connect(
    m_signalModel,
    &QAbstractItemModel::dataChanged,
    this,
    [this](
      const QModelIndex&,
      const QModelIndex&,
      const QList<int>& roles)
    {
      if (!roles.contains(Qt::CheckStateRole))
        return;

      if (!MonitorSettingsLoader::saveSignals(
            m_configFile,
            m_configurationId,
            m_signalModel->selectedSignals()))
      {
        statusBar()->showMessage(
          tr("Не удалось сохранить выбранные сигналы"));
      }

      if (m_monitorState == MonitorState::Stopped)
        updateChartSignals();
    });


  setWindowTitle(
    QString("DataMonitor [%1]")
      .arg(database.databaseName()));

}

void MainWindow::createToolbar()
{
  m_toolBar = addToolBar(tr("Основная"));
  m_toolBar->setIconSize(QSize(24, 24));

  m_toolBar->setMovable(false);

  m_configurationSelector =
    new ConfigurationSelector(m_database, this);

  m_configurationSelector->setMinimumWidth(200);

  m_configurationSelector->setSizePolicy(
    QSizePolicy::Fixed,
    QSizePolicy::Preferred);

  m_monitorAction = m_toolBar->addAction(
    themedSvgIcon(":/images/sensor.svg"),
    tr("Мониторинг"));

  m_monitorAction->setCheckable(true);
  m_monitorAction->setChecked(false);

  m_toolBar->addSeparator();

  m_pauseAction = m_toolBar->addAction(
    themedSvgIcon(":/images/pause.svg"),
    tr("Пауза"));

  m_pauseAction->setCheckable(true);
  m_pauseAction->setChecked(false);
  m_pauseAction->setEnabled(false);
  m_pauseAction->setToolTip(
    tr("Приостановить отображение графика"));

  m_toolBar->addSeparator();

  m_themeAction = m_toolBar->addAction(
    themedSvgIcon(":/images/theme.svg"),
    tr("Сменить тему"));

  m_themeAction->setToolTip(
    tr("Переключить светлую/тёмную тему"));

  m_toolBar->addSeparator();

  m_toolBar->addWidget(
    new QLabel(tr("Эксперимент:"), m_toolBar));

  m_toolBar->addWidget(
    m_configurationSelector);

  auto* spacer = new QWidget(m_toolBar);

  spacer->setSizePolicy(
    QSizePolicy::Expanding,
    QSizePolicy::Preferred);

  m_toolBar->addWidget(spacer);

  m_toolBar->addWidget(
    new QLabel(tr("Интервал:"), m_toolBar));

  m_chartIntervalCombo = new QComboBox(this);

  for (int seconds : {5, 10, 30, 60})
  {
    m_chartIntervalCombo->addItem(
      tr("%1 с").arg(seconds),
      seconds);
  }

  m_toolBar->addWidget(m_chartIntervalCombo);

  m_toolBar->addSeparator();

  m_toolBar->addWidget(
    new QLabel(tr("Ось Y:"), this));

  m_yAxisModeCombo = new QComboBox(this);

  m_yAxisModeCombo->addItem(
    tr("Одна"),
    static_cast<int>(
      SignalChartWidget::YAxisMode::Single));

  m_yAxisModeCombo->addItem(
    tr("Несколько"),
    static_cast<int>(
      SignalChartWidget::YAxisMode::Multiple));

  m_toolBar->addWidget(m_yAxisModeCombo);

  connect(
    m_monitorAction,
    &QAction::toggled,
    this,
    [this](bool checked)
    {
      if (checked) {
        startMonitoring();
      } else {
        stopMonitoring();
      }
    });

  connect(
    m_chartIntervalCombo,
    &QComboBox::currentIndexChanged,
    this,
    [this](int index)
    {
      if (index < 0)
        return;

      m_signalChart->setWindowSeconds(
        m_chartIntervalCombo->itemData(index).toInt());
    });

  connect(
    m_yAxisModeCombo,
    &QComboBox::currentIndexChanged,
    this,
    [this](int index)
    {
      if (index < 0)
        return;

      const int mode =
        m_yAxisModeCombo->itemData(index).toInt();

      m_signalChart->setYAxisMode(
        static_cast<SignalChartWidget::YAxisMode>(
          mode));

      if (!MonitorSettingsLoader::saveChartYAxisMode(
            m_configFile,
            m_configurationId,
            mode))
      {
        statusBar()->showMessage(
          tr("Не удалось сохранить режим осей Y"));
      }
    });

  connect(
    m_pauseAction,
    &QAction::toggled,
    this,
    [this](bool paused)
    {
      m_signalChart->setPaused(paused);

      m_pauseAction->setText(
        paused ? tr("Продолжить") : tr("Пауза"));

      m_pauseAction->setToolTip(
        paused
          ? tr("Продолжить отображение графика")
          : tr("Приостановить отображение графика"));
    });

  connect(
    m_themeAction,
    &QAction::triggered,
    this,
    &toggleColorScheme);
}

void MainWindow::createClient()
{
  QSettings settings(
    m_configFile,
    QSettings::IniFormat);

  const QString host =
    settings.value(
      "Server/Host",
      "127.0.0.1").toString();

  if (!m_serverAddress.setAddress(host))
  {
    QMessageBox::critical(
      this,
      tr("DataMonitor"),
      tr("Некорректный адрес DataServer: %1")
        .arg(host));
  }

  m_client = new ClientConnection(this);

  connect(
    m_client,
    &ClientConnection::subscribed,
    this,
    [this](SubscriptionId id)
    {
      const auto& ids = m_client->signalIds();

      std::vector<QString> names;
      names.reserve(ids.size());

      for (SignalId signalId : ids)
        names.push_back(
          m_signalModel->signalName(signalId));

      m_signalChart->setSignals(ids, names);

      setMonitorState(MonitorState::Running);

      statusBar()->showMessage(
        tr("Подписка активна: %1").arg(id.value));
    });

  connect(
    m_client,
    &ClientConnection::unsubscribed,
    this,
    [this]()
    {
      setMonitorState(MonitorState::Stopped);
      m_signalModel->clearValues();
      m_signalChart->clear();

      statusBar()->showMessage(
        tr("Подписка отменена"));
    });

  connect(
    m_client,
    &ClientConnection::dataReceived,
    this,
    [this](
      SubscriptionId id,
      uint64_t timestamp,
      std::vector<double> values)
    {
      if (id != m_client->subscriptionId())
        return;

      const auto& ids = m_client->signalIds();

      m_signalModel->updateValues(
        ids,
        values);

      m_signalChart->appendData(
        timestamp,
        values);
    });

  connect(
    m_client,
    &ClientConnection::dataTimeout,
    this,
    [this]()
    {
      m_signalModel->clearValues();
      // при dataTimeout лучше очищать только точки, сохраняя серии, а при новой подписке setSignals() создаст их заново.
      m_signalChart->clear();

      statusBar()->showMessage(
        tr("Нет данных от DataServer"));
    });

  connect(
    m_client,
    &ClientConnection::errorOccurred,
    this,
    [this](const QString& message)
    {
      // - При обычной ошибке первоначальной подписки состояние Subscribing переводится в Stopped.
      // - При постоянной ошибке восстановления failRecovery() закрывает соединение, поэтому !m_client->isOpen() будет true.
      // - При временных ошибках восстановления errorOccurred() не вызывается, мониторинг остаётся включённым.
      // - При ошибке отписки сохраняется прежнее поведение.
      qWarning().noquote() << message;

      if (m_monitorState == MonitorState::Subscribing ||
          (m_monitorState == MonitorState::Running &&
           !m_client->isOpen()))
      {
        setMonitorState(MonitorState::Stopped);
        m_signalModel->clearValues();
        m_signalChart->clear();
      }
      else if (m_monitorState == MonitorState::Unsubscribing)
      {
        // Отмена не подтверждена.
        // Подписка может продолжать существовать.
        setMonitorState(MonitorState::Running);
      }

      statusBar()->showMessage(message);
    });

  connect(
    m_signalChart,
    &SignalChartWidget::windowSecondsChanged,
    this,
    [this](int seconds)
    {
      if (!MonitorSettingsLoader::saveChartInterval(
            m_configFile,
            m_configurationId,
            seconds))
      {
        statusBar()->showMessage(
          tr("Не удалось сохранить интервал графика"));
      }
    });

  connect(
    m_signalChart,
    &SignalChartWidget::hiddenSignalsChanged,
    this,
    [this]()
    {
      if (!MonitorSettingsLoader::saveHiddenChartSignals(
            m_configFile,
            m_configurationId,
            m_signalChart->hiddenSignals()))
      {
        statusBar()->showMessage(
          tr("Не удалось сохранить видимость линий графика"));
      }
    });

}

bool MainWindow::loadConfiguration(
  ConfigurationId id)
{
  ConfigurationRepository repository(m_database);

  ConfigurationInfo info;

  if (!repository.configuration(id, info))
    return false;

  std::vector<SignalDefinition> signalIds;

  if (!repository.signalDefinitions(id, signalIds))
    return false;

  m_configurationId = id;
  m_serverPort = info.udpPort;

  m_signalModel->setSignals(signalIds);

  const int chartInterval =
    MonitorSettingsLoader::loadChartInterval(
      m_configFile,
      id);

  {
    QSignalBlocker blocker(m_signalChart);

    m_signalChart->setWindowSeconds(
      chartInterval);
  }

  {
    QSignalBlocker blocker(m_chartIntervalCombo);

    const int index =
      m_chartIntervalCombo->findData(chartInterval);

    if (index >= 0)
      m_chartIntervalCombo->setCurrentIndex(index);
  }

  const int axisMode =
    MonitorSettingsLoader::loadChartYAxisMode(
      m_configFile,
      id);

  {
    QSignalBlocker blocker(m_yAxisModeCombo);

    const int index =
      m_yAxisModeCombo->findData(axisMode);

    if (index >= 0)
      m_yAxisModeCombo->setCurrentIndex(index);
  }

  m_signalChart->setYAxisMode(
    static_cast<SignalChartWidget::YAxisMode>(
      axisMode));

  const auto hiddenSignals =
    MonitorSettingsLoader::loadHiddenChartSignals(
      m_configFile,
      id);

  {
    QSignalBlocker blocker(m_signalChart);

    m_signalChart->setHiddenSignals(
      hiddenSignals);
  }

  const auto selected =
    MonitorSettingsLoader::loadSignals(
      m_configFile,
      id);

  {
    QSignalBlocker blocker(m_signalModel);

    m_signalModel->setSelectedSignals(selected);
  }

  updateChartSignals();

  return true;
}

bool MainWindow::openConnection()
{
  if (m_serverAddress.isNull() ||
      m_serverPort == 0)
  {
    statusBar()->showMessage(
      tr("Не заданы параметры DataServer"));

    return false;
  }

  m_client->close();

  if (!m_client->open(
        m_serverAddress,
        m_serverPort))
  {
    statusBar()->showMessage(
      tr("Не удалось открыть UDP-сокет"));

    return false;
  }

  statusBar()->showMessage(
    tr("UDP открыт: %1:%2")
      .arg(m_serverAddress.toString())
      .arg(m_serverPort),
    5000);

  return true;
}

void MainWindow::startMonitoring()
{
  if (m_monitorState != MonitorState::Stopped)
    return;

  const auto signalIds =
    m_signalModel->selectedSignals();

  if (signalIds.empty())
  {
    QMessageBox::warning(
      this,
      tr("DataMonitor"),
      tr("Не выбраны сигналы для мониторинга."));

    QSignalBlocker blocker(m_monitorAction);
    m_monitorAction->setChecked(false);

    return;
  }

  if (!m_client->isOpen() &&
      !m_client->open(m_serverAddress, m_serverPort))
  {
    setMonitorState(MonitorState::Stopped);

    QMessageBox::warning(
      this,
      tr("DataMonitor"),
      tr("Не удалось открыть UDP-сокет."));

    return;
  }

  if (!m_client->subscribe(
        signalIds,
        PublishRate::Hz10))
  {
    setMonitorState(MonitorState::Stopped);

    QMessageBox::warning(
      this,
      tr("DataMonitor"),
      tr("Не удалось отправить запрос подписки."));

    return;
  }

  setMonitorState(MonitorState::Subscribing);

  statusBar()->showMessage(
    tr("Ожидание подтверждения подписки..."));

  updateControls();
}

void MainWindow::stopMonitoring()
{
  m_pauseAction->setChecked(false);

  if (m_client->isRecovering())
  {
    m_client->cancelRecovery();

    m_signalModel->clearValues();
    m_signalChart->clear();

    setMonitorState(MonitorState::Stopped);

    statusBar()->showMessage(
      tr("Мониторинг остановлен"));

    return;
  }

  if (m_monitorState != MonitorState::Running)
    return;

  if (!m_client->unsubscribe())
  {
    statusBar()->showMessage(
      tr("Не удалось отправить запрос отмены подписки"));

    setMonitorState(MonitorState::Running);
    return;
  }

  setMonitorState(MonitorState::Unsubscribing);

  statusBar()->showMessage(
    tr("Ожидание отмены подписки..."));
}

void MainWindow::updateControls()
{
  const bool stopped =
    m_monitorState == MonitorState::Stopped;

  const bool pending =
    m_monitorState == MonitorState::Subscribing ||
    m_monitorState == MonitorState::Unsubscribing;

  m_monitorAction->setEnabled(!pending);

  m_pauseAction->setEnabled(
    m_monitorState == MonitorState::Running);

  m_configurationSelector->setEnabled(stopped);

  m_signalView->setEnabled(stopped);
}

void MainWindow::updateChartSignals()
{
  const auto ids =
    m_signalModel->selectedSignals();

  std::vector<QString> names;
  names.reserve(ids.size());

  for (SignalId signalId : ids)
  {
    names.push_back(
      m_signalModel->signalName(signalId));
  }

  m_signalChart->setSignals(ids, names);
}

void MainWindow::setMonitorState(
  MonitorState state)
{
  m_monitorState = state;

  QSignalBlocker blocker(m_monitorAction);

  m_monitorAction->setChecked(
    state == MonitorState::Running ||
    state == MonitorState::Unsubscribing);

  updateControls();
}

}
