#include "archivedescriptionbuilder.h"
#include "archivemanager.h"
#include "configurationrepository.h"
#include "lcarddatasource.h"
#include "ltr11configurationbuilder.h"
#include "ltr11module.h"
#include "schedulerclock.h"
#include "systemclock.h"
#include "systemconfiguration.h"
#include "dataserver.h"
#include "testdatastreamsource.h"
#include "udpsender.h"
#include "logger.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QSettings>

static bool get_db(
  const QCoreApplication& app,
  QSqlDatabase& database,
  qds::ConfigurationId& configuration,
  std::filesystem::path& archiveDirectory,
  std::filesystem::path& logDirectory)
{
  QCommandLineParser parser;
  parser.setApplicationDescription(
    "DataServer Configuration Editor");

  parser.addHelpOption();

  QCommandLineOption configOption(
    "config",
    "Configuration INI file",
    "file",
    "dataserver.ini");

  parser.addOption(configOption);
  parser.process(app);

  const QString configFile =
    parser.value(configOption);

  if (!QFileInfo::exists(configFile))
  {
    qCritical()
      << "Configuration error"
      << "Configuration file not found:\n"
      << configFile;

    return false;
  }

  QSettings settings(
    configFile,
    QSettings::IniFormat);

  settings.beginGroup("Database");

  const QString plugin =
    settings.value(
              "Plugin",
              "QMARIADB").toString();

  const QString host =
    settings.value(
              "Host",
              "127.0.0.1").toString();

  const int port =
    settings.value(
              "Port",
              3306).toInt();

  const QString databaseName =
    settings.value(
              "Name").toString();

  const QString userName =
    settings.value(
              "User").toString();

  const QString password =
    settings.value(
              "Password").toString();

  settings.endGroup();

  if (settings.status() != QSettings::NoError ||
      databaseName.isEmpty() ||
      userName.isEmpty() ||
      port < 1 ||
      port > 65535)
  {
    qCritical()
      << "Configuration error"
      << "Invalid database settings:\n"
      << configFile;

    return false;
  }

  settings.beginGroup("Server");

  bool idOk = false;

  const auto configurationId =
    settings.value("ConfigurationId")
      .toUInt(&idOk);

  settings.endGroup();

  if (!idOk || configurationId == 0)
  {
    qCritical() << "Invalid ConfigurationId";
    return false;
  }

  settings.beginGroup("Archive");

  const auto archive =
    settings.value("Directory")
      .toString();

  settings.endGroup();

  if (archive.isEmpty())
  {
    qCritical() << "Archive Directory error";
    return false;
  }

  settings.beginGroup("Log");

  const auto log =
    settings.value("Directory")
      .toString();

  settings.endGroup();

  if (log.isEmpty())
  {
    qCritical() << "Log Directory error";
    return false;
  }


  QSqlDatabase db =
    QSqlDatabase::addDatabase(plugin);

  db.setHostName(host);
  db.setPort(port);
  db.setDatabaseName(databaseName);
  db.setUserName(userName);
  db.setPassword(password);

  if (!db.open())
  {
    qCritical()
      << "Database error"
      << db.lastError().text();

    return false;
  }

  database = std::move(db);

  configuration = qds::ConfigurationId{configurationId};

  archiveDirectory = std::filesystem::path(archive.toStdString());
  if (!std::filesystem::is_directory(archiveDirectory))
  {
    qCritical() << "Archive Directory error";
    return false;
  }

  logDirectory = std::filesystem::path(log.toStdString());
  if (!std::filesystem::is_directory(logDirectory))
  {
    qCritical() << "Log Directory error";
    return false;
  }

  return true;
}


int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  QSqlDatabase database;
  qds::ConfigurationId configuration;
  std::filesystem::path archivePath;
  std::filesystem::path logPath;

  if (!get_db(
        app,
        database,
        configuration,
        archivePath,
        logPath))
    return -1;

  qds::SystemConfiguration cfg;
  qds::SensorRepository sensors;
  qds::CalibrationRepository calibrations;

  qds::ConfigurationRepository repository(database);

  if (!repository.load(configuration, cfg))
    return 1;

  if (!repository.loadSensors(sensors))
    return 1;

  if (!repository.loadCalibrations(cfg, calibrations))
    return 1;

  qds::SystemClock systemClock;
  qds::SchedulerClock clock(systemClock);

  qds::ArchiveDescriptionBuilder builder;
  qds::ArchiveDescription description;

  if (!builder.build(cfg, description))
    return 1;

  qds::SignalMemoryLayout layout;
  layout.build(cfg);

  qds::ArchiveManager archive;

  if (!archive.initialize(
        archivePath,
        description,
        layout))
    return 1;


  qds::DataStreamSourceFactory factory;

  if (!factory.registerType(
        qds::ModuleType::Test,
        [](const qds::ModuleRuntimeConfiguration&,
           qds::IClock&,
           qds::IDataBlockSink&,
           qds::IDataStreamEventSink&)
        {
          return std::make_unique<qds::TestDataStreamSource>();
        }))
    return -1;

  if (!factory.registerType(
    qds::ModuleType::LTR11,
    [](const qds::ModuleRuntimeConfiguration& cfg,
       qds::IClock& clock,
       qds::IDataBlockSink& blockSink,
       qds::IDataStreamEventSink& eventSink)
    -> std::unique_ptr<qds::IDataStreamSource>
    {
      qds::Ltr11Configuration configuration;
      qds::Ltr11ConfigurationBuilder builder;

      if (!builder.build(
            cfg,
            configuration))
      {
        return nullptr;
      }

      auto module =
        std::make_unique<qds::Ltr11Module>(
          configuration);

      return
        std::make_unique<qds::LCardDataSource>(
          cfg.module.id,
          static_cast<uint32_t>(
            configuration.channels.size()),
          std::move(module),
          clock,
          &blockSink,
          &eventSink);
        }))
  {
    return -1;
  }

  qds::UdpSender sender;

  qds::Logger logger(logPath, clock);

  qds::DataServer server(
    cfg,
    sensors,
    calibrations,
    factory,
    archive,
    clock,
    sender,
    logger);

  if (!server.start())
  {
    archive.close();
    return 1;
  }

  const int result = app.exec();

  server.stop();
  archive.close();

  return result;
}
