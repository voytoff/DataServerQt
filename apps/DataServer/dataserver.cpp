#include "dataserver.h"
#include "systembuilder.h"
#include "packetdispatcher.h"

namespace qds
{

DataServer::DataServer(
  SystemConfiguration configuration,
  const CalibrationRepository& repository,
  const DataSourceFactory& dataSourceFactory,
  IArchiveWriter& archive,
  ISchedulerClock& clock,
  ISender& sender,
  QObject* parent)
  : QObject(parent)
  , m_configuration(std::move(configuration))
  , m_repository(repository)
  , m_dataSourceFactory(dataSourceFactory)
  , m_archive(archive)
  , m_clock(clock)
  , m_sender(sender)
{
  connect(
    &m_timer,
    &QTimer::timeout,
    this,
    &DataServer::onTimer);

  m_timer.setInterval(1);
}

bool DataServer::start()
{
  if (m_running)
    return true;

  if (m_configuration.udpPort() == 0)
    return false;

  SystemBuilder builder;

  if (!builder.build(
        m_configuration,
        m_dataSourceFactory,
        m_repository,
        m_runtime))
  {
    cleanup();
    return false;
  }

  m_publisher =
    std::make_unique<Publisher>(
      m_runtime.layout,
      m_subscriptions,
      m_sender,
      1000);

  m_dispatcher =
    std::make_unique<PacketDispatcher>(
      m_configuration,
      m_subscriptions,
      m_sender);

  m_udpServer =
    std::make_unique<UdpServer>(
      *m_dispatcher);

  if (!m_runtime.engine->initialize(
        m_runtime.dataSources,
        *m_runtime.signalProcessor,
        m_runtime.buffers,
        m_archive,
        *m_publisher,
        m_clock))
  {
    cleanup();
    return false;
  }

  if (!m_udpServer->start(
        m_configuration.udpPort()))
  {
    cleanup();
    return false;
  }

  m_running = true;
  m_timer.start();

  return true;
}

void DataServer::stop()
{
  if (!m_running)
    return;

  cleanup();

  m_running = false;
}

bool DataServer::isRunning() const noexcept
{
  return m_running;
}

void DataServer::cleanup()
{
  m_timer.stop();

  if (m_udpServer)
    m_udpServer->stop();

  if (m_runtime.engine)
    m_runtime.engine->stop();

  m_udpServer.reset();
  m_dispatcher.reset();
  m_publisher.reset();

  m_runtime = {};
  m_subscriptions.clear();
}

void DataServer::onTimer()
{
  if (!m_runtime.engine)
  {
    stop();
    return;
  }

  if (!m_runtime.engine->process())
  {
    stop();
    return;
  }
}
/*
void DataServer::rollbackStart()
{
  m_timer.stop();

  if (m_udpServer)
    m_udpServer->stop();

  if (m_runtime.engine)
    m_runtime.engine->stop();

  m_udpServer.reset();
  m_dispatcher.reset();
  m_publisher.reset();

  m_subscriptions.clear();

  m_running = false;
}
*/
}