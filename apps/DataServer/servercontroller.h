#pragma once

#include "archivemanager.h"
#include "dataserver.h"
#include "datastreamsourcefactory.h"
#include "guilogger.h"
#include "logger.h"
#include "schedulerclock.h"
#include "systemclock.h"
#include "udpsender.h"

#include <QObject>
#include <QSqlDatabase>

#include <filesystem>
#include <memory>

namespace qds
{

class ServerController : public QObject
{
  Q_OBJECT

public:
  explicit ServerController(
    const QSqlDatabase& database,
    ConfigurationId configurationId,
    const std::filesystem::path& archiveDirectory,
    const std::filesystem::path& logDirectory,
    QObject* parent = nullptr);

  ~ServerController() override;

  bool start();
  void stop();

  [[nodiscard]]
  bool isRunning() const noexcept;

private:
  bool loadConfiguration();
  bool registerDataSources();
  bool initializeArchive();

private:
  QSqlDatabase m_database;
  ConfigurationId m_configurationId;

  std::filesystem::path m_archiveDirectory;
  std::filesystem::path m_logDirectory;

  SystemConfiguration m_configuration;
  SensorRepository m_sensors;
  CalibrationRepository m_calibrations;

  SystemClock m_systemClock;
  SchedulerClock m_clock;

  DataStreamSourceFactory m_factory;
  ArchiveManager m_archive;
  UdpSender m_sender;

  Logger m_fileLogger;
  GuiLogger m_guiLogger;

  std::unique_ptr<DataServer> m_server;

  bool m_sourcesRegistered = false;


signals:
  void started();
  void stopped();

  void logMessage(
    LogLevel level,
    QString message);
};

}