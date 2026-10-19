#include "servercontroller.h"

#include "archivedescriptionbuilder.h"
#include "configurationrepository.h"
#include "lcarddatasource.h"
#include "ltr11configurationbuilder.h"
#include "ltr11module.h"
#include "signalmemorylayout.h"
#include "testdatastreamsource.h"

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
  , m_fileLogger(m_logDirectory, m_clock)
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
  ConfigurationRepository repository(m_database);

  SystemConfiguration configuration;
  SensorRepository sensors;
  CalibrationRepository calibrations;

  if (!repository.load(
        m_configurationId,
        configuration))
    return false;

  if (!repository.loadSensors(sensors))
    return false;

  if (!repository.loadCalibrations(
        configuration,
        calibrations))
    return false;

  m_configuration = std::move(configuration);
  m_sensors = std::move(sensors);
  m_calibrations = std::move(calibrations);

  return true;
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
        [](const ModuleRuntimeConfiguration& cfg,
           IClock& clock,
           IDataBlockSink& blockSink,
           IDataStreamEventSink& eventSink)
        -> std::unique_ptr<IDataStreamSource>
        {
          Ltr11Configuration configuration;
          Ltr11ConfigurationBuilder builder;

          if (!builder.build(cfg, configuration))
            return nullptr;

          auto module =
            std::make_unique<Ltr11Module>(
              configuration);

          return std::make_unique<LCardDataSource>(
            cfg.module.id,
            static_cast<uint32_t>(
              configuration.channels.size()),
            std::move(module),
            clock,
            &blockSink,
            &eventSink);
        }))
    return false;

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
  layout.build(m_configuration);

  return m_archive.initialize(
    m_archiveDirectory,
    description,
    layout);
}

bool ServerController::start()
{
  if (isRunning())
    return true;

  // После предыдущей остановки освобождаем старые ресурсы.
  m_server.reset();
  m_archive.close();

  if (!loadConfiguration())
    return false;

  if (!m_sourcesRegistered)
  {
    if (!registerDataSources())
      return false;

    m_sourcesRegistered = true;
  }

  if (!initializeArchive())
    return false;

  m_server = std::make_unique<DataServer>(
    m_configuration,
    m_sensors,
    m_calibrations,
    m_factory,
    m_archive,
    m_clock,
    m_sender,
    m_guiLogger);

  if (!m_server->start())
  {
    m_server.reset();
    m_archive.close();
    return false;
  }

  emit started();

  return true;
}

void ServerController::stop()
{
  if (!m_server)
    return;

  const bool wasRunning = m_server->isRunning();

  m_server->stop();
  m_server.reset();

  m_archive.close();

  if (wasRunning)
    emit stopped();
}

bool ServerController::isRunning() const noexcept
{
  return m_server && m_server->isRunning();
}

}