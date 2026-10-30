#include "servercontroller.h"

#include "archivedescriptionbuilder.h"
#include "archivedescriptionwriter.h"
#include "configurationrepository.h"
#include "lcarddatasource.h"
#include "ltr11configurationbuilder.h"
#include "ltr11module.h"
#include "signalmemorylayout.h"
#include "testdatastreamsource.h"

#include <QDir>

namespace qds
{

ServerController::ServerController(
  const QSqlDatabase& database,
  ConfigurationId configurationId,
  const std::filesystem::path& archiveDirectory,
  const std::filesystem::path& logDirectory,
  QObject* parent)
  : QObject(parent)
  , m_database(database)
  , m_configurationId(configurationId)
  , m_archiveDirectory(archiveDirectory)
  , m_logDirectory(logDirectory)
  , m_clock(m_systemClock)
  , m_fileLogger(m_logDirectory, m_systemClock)
  , m_guiLogger(m_fileLogger)
{
  connect(
    &m_guiLogger,
    &GuiLogger::messageReceived,
    this,
    &ServerController::logMessage);
}

ServerController::~ServerController()
{
  stop();
}

bool ServerController::loadConfiguration()
{
  if (isRunning())
    return false;

  m_configurationLoaded = false;

  m_guiLogger.info("Загрузка конфигурации");

  ConfigurationRepository repository(m_database);

  SystemConfiguration configuration;
  SensorRepository sensors;
  CalibrationRepository calibrations;

  if (!repository.load(
        m_configurationId,
        configuration))
  {
    m_guiLogger.error("Не удалось загрузить конфигурацию эксперимента");
    return false;
  }

  if (!repository.loadSensors(sensors))
  {
    m_guiLogger.error("Не удалось загрузить датчики");
    return false;
  }

  if (!repository.loadCalibrations(
        configuration,
        calibrations))
  {
    m_guiLogger.error("Не удалось загрузить калибровки");
    return false;
  }

  m_configuration = std::move(configuration);
  m_sensors = std::move(sensors);
  m_calibrations = std::move(calibrations);

  m_configurationLoaded = true;

  return true;
}

std::optional<DataStreamSourceStatistics>
ServerController::statistics(ModuleId id) const noexcept
{
  if (!m_server)
    return std::nullopt;

  return m_server->statistics(id);
}

std::optional<DataStreamSourceState> ServerController::state(
  ModuleId id) const noexcept
{
  if (!m_server)
    return std::nullopt;

  return m_server->state(id);
}

std::optional<bool> ServerController::isRunning(ModuleId id) const noexcept
{
  if (!m_server)
    return std::nullopt;

  return m_server->isRunning(id);
}

const std::filesystem::path &ServerController::currentArchiveDirectory() const noexcept
{
  return m_currentArchiveDirectory;
}

bool ServerController::registerDataSources()
{
  if (!m_factory.registerType(
        ModuleType::Test,
        [](const ModuleRuntimeConfiguration&,
           IClock&,
           IDataBlockSink&,
           IDataStreamEventSink&)
        {
          return std::make_unique<TestDataStreamSource>();
        }))
    return false;

  if (!m_factory.registerType(
        ModuleType::LTR11,
        [this](const ModuleRuntimeConfiguration& cfg,
           IClock& clock,
           IDataBlockSink& blockSink,
           IDataStreamEventSink& eventSink)
        -> std::unique_ptr<IDataStreamSource>
        {
          Ltr11Configuration configuration;
          Ltr11ConfigurationBuilder builder;

          if (!builder.build(
                cfg,
                configuration))
          {
            m_guiLogger.error(
              "Ошибка конфигурации модуля LTR11 "
              "(ModuleId: " +
              std::to_string(cfg.module.id.value) +
              "): " +
              builder.lastError());

            return nullptr;
          }

          auto module =
            std::make_unique<Ltr11Module>(
              configuration, m_guiLogger);

          return std::make_unique<LCardDataSource>(
            cfg.module.id,
            static_cast<uint32_t>(
              configuration.channels.size()),
            std::move(module),
            clock,
            m_guiLogger,
            &blockSink,
            &eventSink);
        }))
  {
    return false;
  }

  return true;
}

bool ServerController::initializeArchive()
{
  ArchiveDescriptionBuilder builder;
  ArchiveDescription description;

  if (!builder.build(
        m_configuration,
        description))
    return false;

  SignalMemoryLayout layout;
  if (!layout.build(m_configuration))
  {
    m_guiLogger.error(
      "Ошибка построения памяти сигналов: " +
      layout.lastError());

    return false;
  }

  const QString timestamp =
    QDateTime::currentDateTime().toString(
      "yyyyMMdd_HHmmss");

  const auto sessionDirectory =
    m_archiveDirectory /
    timestamp.toStdString();

  std::error_code ec;

  std::filesystem::create_directories(
    sessionDirectory, ec);

  if (ec)
  {
    m_guiLogger.error(
      "Не удалось создать каталог архива: " +
      sessionDirectory.string());

    return false;
  }

  if (!m_archive.initialize(
        sessionDirectory,
        description,
        layout))
  {
    m_guiLogger.error(
      "Не удалось инициализировать архив");

    m_archive.close();
    return false;
  }

  ArchiveDescriptionWriter writer;

  if (!writer.write(
        (sessionDirectory / "description.json").string(),
        description))
  {
    m_guiLogger.error(
      "Не удалось записать описание архива");

    m_archive.close();
    return false;
  }

  const QString path =
    QDir::toNativeSeparators(
      QString::fromStdWString(
        sessionDirectory.wstring()));

  m_guiLogger.info(
    "Создан архив: " +
    path.toStdString());

  m_currentArchiveDirectory = sessionDirectory;

  return true;
}

ConfigurationId ServerController::configurationId() const noexcept
{
  return m_configurationId;
}

bool ServerController::configurationLoaded() const noexcept
{
  return m_configurationLoaded;
}

bool ServerController::setConfigurationId(
  ConfigurationId id)
{
  if (isRunning() || id.value == 0)
    return false;

  if (m_configurationId == id)
    return true;

  const ConfigurationId previousId =
    m_configurationId;

  const bool previousLoaded =
    m_configurationLoaded;

  m_configurationId = id;
  m_configurationLoaded = false;

  if (!loadConfiguration())
  {
    m_configurationId = previousId;
    m_configurationLoaded = previousLoaded;
    return false;
  }

  return true;
}

const std::filesystem::path&
ServerController::archiveDirectory() const noexcept
{
  return m_archiveDirectory;
}

void ServerController::onDataServerStopped()
{
  m_archive.close();

  m_guiLogger.info("Сервер остановлен");

  emit stopped();
}

bool ServerController::start()
{
  if (isRunning())
    return true;

  m_server.reset();
  m_archive.close();

  if (!m_configurationLoaded && !loadConfiguration())
    return false;

  m_guiLogger.info("Регистрация источников данных");

  if (!m_sourcesRegistered)
  {
    if (!registerDataSources())
    {
      m_guiLogger.error("Не удалось зарегистрировать источники данных");
      return false;
    }

    m_sourcesRegistered = true;
  }

  m_guiLogger.info("Инициализация архива");

  if (!initializeArchive())
  {
    m_guiLogger.error("Не удалось инициализировать архив");
    return false;
  }

  m_guiLogger.info("Создание сервера");

  m_server = std::make_unique<DataServer>(
    m_configuration,
    m_sensors,
    m_calibrations,
    m_factory,
    m_archive,
    m_clock,
    m_systemClock,
    m_sender,
    m_guiLogger);

  m_guiLogger.info("Запуск сервера");



/*
  const auto connection = connect(
    m_server.get(),
    &DataServer::stopped,
    this,
    [this]()
    {
      m_archive.close();

      m_guiLogger.info("Сервер остановлен");

      emit stopped();
    },
    Qt::DirectConnection);
*/

  const auto connection = QObject::connect(
    m_server.get(),
    SIGNAL(stopped()),
    this,
    SLOT(onDataServerStopped()));

  if (!connection)
  {
    m_guiLogger.error(
      "Не удалось подключить сигнал DataServer::stopped");

    return false;
  }

  if (!m_server->start())
  {
    m_guiLogger.error("Не удалось запустить сервер");

    m_server.reset();
    m_archive.close();

    return false;
  }

  m_guiLogger.info("Сервер запущен");

  emit started();

  return true;
}

void ServerController::stop()
{
  if (!m_server)
    return;

  if (m_server->isRunning())
    m_guiLogger.info("Остановка сервера");

  m_server->stop();
  m_server.reset();

  m_archive.close();
}

bool ServerController::isRunning() const noexcept
{
  return m_server && m_server->isRunning();
}

const SystemConfiguration&
ServerController::configuration() const noexcept
{
  return m_configuration;
}

}