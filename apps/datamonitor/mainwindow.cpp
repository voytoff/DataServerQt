#include "mainwindow.h"

#include "applicationsettingsloader.h"
#include "basetableview.h"
#include "configurationrepository.h"
#include "configurationselector.h"
#include "monitorsettingsloader.h"

#include <QSignalBlocker>
#include <QHeaderView>
#include <QMessageBox>
#include <QSizePolicy>
#include <QToolBar>
#include <QLabel>
#include <QSettings>
#include <QStatusBar>
#include <QDebug>

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
    new BaseTableView(this);

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

  setCentralWidget(m_signalView);

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
        qWarning()
        << "Не удалось сохранить выбранные сигналы";
      }
    });


  setWindowTitle(
    QString("DataMonitor [%1]")
      .arg(database.databaseName()));

}

void MainWindow::createToolbar()
{
  m_toolBar = addToolBar(tr("Основная"));

  m_toolBar->setMovable(false);

  m_configurationSelector =
    new ConfigurationSelector(m_database, this);

  m_configurationSelector->setMinimumWidth(220);

  m_configurationSelector->setSizePolicy(
    QSizePolicy::Fixed,
    QSizePolicy::Preferred);

  m_monitorAction = m_toolBar->addAction(
    tr("Мониторинг"));

  m_monitorAction->setCheckable(true);
  m_monitorAction->setChecked(false);

  m_toolBar->addSeparator();

  m_toolBar->addWidget(
    new QLabel(tr("Эксперимент:"), m_toolBar));

  m_toolBar->addWidget(
    m_configurationSelector);

  connect(
    m_monitorAction,
    &QAction::toggled,
    this,
    [this](bool checked)
    {
      if (checked)
      {
        startMonitoring();
      }
      else
      {
        stopMonitoring();
      }
    });}

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
      statusBar()->showMessage(
        tr("Подписка активна: %1").arg(id.value));
    });

  connect(
    m_client,
    &ClientConnection::unsubscribed,
    this,
    [this]()
    {
      statusBar()->showMessage(
        tr("Подписка отменена"));
    });

  connect(
    m_client,
    &ClientConnection::errorOccurred,
    this,
    [this](const QString& message)
    {
      qWarning().noquote() << message;
      statusBar()->showMessage(message);
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

  const auto selected =
    MonitorSettingsLoader::loadSignals(
      m_configFile,
      id);

  {
    QSignalBlocker blocker(m_signalModel);

    m_signalModel->setSelectedSignals(selected);
  }

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

  if (!m_client->isOpen())
  {
    QMessageBox::warning(
      this,
      tr("DataMonitor"),
      tr("UDP-сокет не открыт."));

    QSignalBlocker blocker(m_monitorAction);
    m_monitorAction->setChecked(false);

    return;
  }

  if (!m_client->subscribe(
        signalIds,
        PublishRate::Hz10))
  {
    QMessageBox::warning(
      this,
      tr("DataMonitor"),
      tr("Не удалось отправить запрос подписки."));

    QSignalBlocker blocker(m_monitorAction);
    m_monitorAction->setChecked(false);

    return;
  }

  statusBar()->showMessage(
    tr("Ожидание подтверждения подписки..."));

  updateControls();
}

void MainWindow::stopMonitoring()
{
  if (!m_client->unsubscribe())
  {
    QMessageBox::warning(
      this,
      tr("DataMonitor"),
      tr("Не удалось отправить запрос отмены подписки."));

    QSignalBlocker blocker(m_monitorAction);
    m_monitorAction->setChecked(true);

    return;
  }

  statusBar()->showMessage(
    tr("Ожидание отмены подписки..."));

  updateControls();
}

void MainWindow::updateControls()
{
  const bool monitoring =
    m_monitorAction->isChecked();

  m_configurationSelector->setEnabled(
    !monitoring);

  m_signalView->setEnabled(
    !monitoring);
}

}
