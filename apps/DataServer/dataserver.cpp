#include "dataserver.h"
#include "systembuilder.h"
#include "packetdispatcher.h"

namespace qds
{

DataServer::DataServer(
  SystemConfiguration configuration,
  const DataSourceFactory& dataSourceFactory,
  IArchiveWriter& archive,
//  IFramePublisher& publisher,
  ISchedulerClock& clock,
  QObject* parent)
  : QObject(parent)
  , m_configuration(std::move(configuration))
  , m_dataSourceFactory(dataSourceFactory)
  , m_archive(archive)
  , m_clock(clock)
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

  SystemBuilder builder;

  if (!builder.build(
        m_configuration,
        m_dataSourceFactory,
        m_archive,
        *m_publisher,
        m_clock,
        m_runtime))
  {
    return false;
  }

  m_publisher =
    std::make_unique<Publisher>(
      m_runtime.layout,
      m_subscriptions,
      m_sender,
      1000);

  m_runtime.engine->initialize(
    m_runtime.dataSources,
    *m_runtime.signalProcessor,
    m_runtime.buffers,
    m_archive,
    *m_publisher,
    m_clock);

  m_dispatcher =
    std::make_unique<PacketDispatcher>(
      m_configuration,
      m_subscriptions,
      m_sender);

  m_udpServer =
    std::make_unique<UdpServer>(
      *m_dispatcher);

  m_running = true;
  m_timer.start();

  return true;
}

void DataServer::stop()
{
  if (!m_running)
    return;

  m_timer.stop();
  m_running = false;
}

void DataServer::onTimer()
{
  if (!m_runtime.engine->process())
  {
    stop();
    return;
  }
}

}