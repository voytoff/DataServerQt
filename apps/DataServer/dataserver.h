#pragma once

#include "datastreamsourcestatistics.h"
#include "moduleconfigurationvalidatorregistry.h"
#include "packetdispatcher.h"
#include "publisher.h"
#include "runtimesystem.h"
#include "subscriptionmanager.h"
#include "systemconfiguration.h"
#include "iarchivewriter.h"
#include "ischedulerclock.h"
#include "isender.h"
#include "ilogger.h"
#include "udpserver.h"
#include "udpsender.h"

#include <QObject>
#include <QTimer>
#include <optional>
#include <unordered_map>

namespace qds
{

class DataServer : public QObject
{
  Q_OBJECT

public:

  explicit DataServer(
    SystemConfiguration configuration,
    const SensorRepository& sensors,
    const CalibrationRepository& calibrations,
    const DataStreamSourceFactory& dataSourceFactory,
    const ModuleConfigurationValidatorRegistry& moduleValidators,
    IArchiveWriter& archive,
    ISchedulerClock& clock,
    IClock& sourceClock,
    ISender& sender,
    ILogger& logger,
    QObject* parent = nullptr);

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

  [[nodiscard]]
  std::size_t subscriptionCount() const noexcept;

private:
  void onTimer();
  void cleanup();

  void cleanupExpiredSubscriptions();

private:
  SystemConfiguration m_configuration;
  SensorRepository m_sensors;
  CalibrationRepository m_calibrations;

  const DataStreamSourceFactory& m_dataSourceFactory;
  const ModuleConfigurationValidatorRegistry& m_moduleValidators;

  IArchiveWriter& m_archive;
  ISchedulerClock& m_clock;
  IClock& m_sourceClock;
  ISender& m_sender;
  ILogger& m_logger;

  std::unique_ptr<RuntimeSystem> m_runtime;

  SubscriptionManager m_subscriptions;

  std::unique_ptr<Publisher> m_publisher;
  std::unique_ptr<PacketDispatcher> m_dispatcher;
  std::unique_ptr<UdpServer> m_udpServer;

  std::unordered_map<uint32_t, DataStreamSourceState> m_lastModuleStates;

  QTimer m_timer; // основной таймер рабочего цикла

  bool m_running = false;

  // данные очистки подписок
  static constexpr int SubscriptionCleanupIntervalMs = 5000;
  QTimer m_subscriptionCleanupTimer;

signals:
  void started();
  void stopped();

};

}