#include "dataserver.h"
#include "systembuilder.h"
#include "packetdispatcher.h"

namespace qds
{

namespace
{

constexpr uint32_t BaseFrameFrequency = 1000;
constexpr int EngineTimerIntervalMs = 1;

}

DataServer::DataServer(
  SystemConfiguration configuration,
  const SensorRepository& sensors,
  const CalibrationRepository& calibrations,
  const DataStreamSourceFactory& dataSourceFactory,
  IArchiveWriter& archive,
  ISchedulerClock& clock,
  ISender& sender,
  ILogger& logger,
  QObject* parent)
  : QObject(parent)
  , m_configuration(std::move(configuration))
  , m_sensors(sensors)
  , m_calibrations(calibrations)
  , m_dataSourceFactory(dataSourceFactory)
  , m_archive(archive)
  , m_clock(clock)
  , m_sender(sender)
  , m_logger(logger)
{
  connect(
    &m_timer,
    &QTimer::timeout,
    this,
    &DataServer::onTimer);

  m_timer.setParent(this);
  m_timer.setInterval(EngineTimerIntervalMs);
}

bool DataServer::start()
{
  if (m_running)
    return true;

  if (m_configuration.udpPort() == 0)
  {
    m_logger.error("Invalid UDP port");
    return false;
  }

  SystemBuilder builder;

  m_runtime =
    builder.build(
      m_configuration,
      m_dataSourceFactory,
      m_sensors,
      m_calibrations,
      m_clock,
      m_archive,
      m_logger);

  if (!m_runtime)
  {
    m_logger.error("SystemBuilder failed");
    cleanup();
    return false;
  }

  m_publisher =
    std::make_unique<Publisher>(
      m_runtime->layout,
      m_subscriptions,
      m_sender,
      BaseFrameFrequency);

  m_dispatcher =
    std::make_unique<PacketDispatcher>(
      m_configuration,
      m_subscriptions,
      m_sender);

  m_udpServer =
    std::make_unique<UdpServer>(
      *m_dispatcher);

  if (!m_runtime->engine->initialize(
        m_runtime->buffers,
        *m_publisher))
  {
    m_logger.error("DataEngine initialization failed");
    cleanup();
    return false;
  }

  m_runtime->queue.start();

  if (!m_runtime->streamWorker->start())
  {
    m_logger.error("StreamWorker start failed");
    cleanup();
    return false;
  }

  if (!m_runtime->dataSources.start())
  {
    m_logger.error("DataSources start failed");
    cleanup();
    return false;
  }

  if (!m_runtime->engine->start())
  {
    m_logger.error("DataEngine start failed");
    cleanup();
    return false;
  }

  if (!m_udpServer->start(
        m_configuration.udpPort()))
  {
    m_logger.error("UDP server start failed");
    cleanup();
    return false;
  }

  m_running = true;
  m_timer.start();

  emit started();

  return true;
}

void DataServer::stop()
{
  cleanup();
}

bool DataServer::isRunning() const noexcept
{
  return m_running;
}

std::optional<DataStreamSourceStatistics>
DataServer::statistics(ModuleId id) const noexcept
{
  if (!m_runtime)
    return std::nullopt;

  return m_runtime->dataSources.statistics(id);
}

void DataServer::cleanup()
{
  m_timer.stop();

  if (m_udpServer)
    m_udpServer->stop();

  if (m_runtime)
  {
    m_runtime->dataSources.stop();

    m_runtime->queue.stop();

    if (m_runtime->streamWorker)
      m_runtime->streamWorker->join();

    if (m_runtime->engine)
      m_runtime->engine->stop();
  }

  m_udpServer.reset();
  m_dispatcher.reset();
  m_publisher.reset();

  m_runtime.reset();

  m_subscriptions.clear();

  m_running = false;

  //if (wasRunning)
    emit stopped();
}

void DataServer::onTimer()
{
  if (!m_runtime || !m_runtime->engine)
  {
    stop();
    return;
  }

  if (!m_runtime->dataSources.isRunning())
  {
    m_logger.error(
      "Data stream source stopped unexpectedly");

    stop();
    return;
  }

  if (!m_runtime->engine->process())
  {
    stop();
    return;
  }
}

}