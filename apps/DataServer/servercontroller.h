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

  [[nodiscard]]
  std::optional<bool> isRunning(
    ModuleId id) const noexcept;

  [[nodiscard]]
  std::optional<DataStreamSourceStatistics> statistics(
    ModuleId id) const noexcept;

  [[nodiscard]]
  std::optional<DataStreamSourceState> state(
    ModuleId id) const noexcept;

  const SystemConfiguration& configuration() const noexcept;

  [[nodiscard]]
  bool loadConfiguration();

  [[nodiscard]]
  const std::filesystem::path& currentArchiveDirectory() const noexcept;

  ConfigurationId configurationId() const noexcept;

  bool configurationLoaded() const noexcept;

  [[nodiscard]]
  bool setConfigurationId(
    ConfigurationId id);

  [[nodiscard]]
  const std::filesystem::path& archiveDirectory() const noexcept;

private:
  bool registerDataSources();
  bool initializeArchive();

private:
  QSqlDatabase m_database;
  ConfigurationId m_configurationId;

  std::filesystem::path m_archiveDirectory;
  std::filesystem::path m_logDirectory;
  std::filesystem::path m_currentArchiveDirectory;

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

  bool m_configurationLoaded = false;

private slots:
  void onDataServerStopped();

signals:
  void started();
  void stopped();

  void logMessage(
    qds::LogLevel level,
    QString message);

  void moduleError(
    qds::ModuleId id);

};

}