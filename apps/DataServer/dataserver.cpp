#include "dataserver.h"
#include "systembuilder.h"

namespace qds
{

DataServer::DataServer(
  SystemConfiguration configuration,
  const DataSourceFactory& dataSourceFactory,
  IArchiveWriter& archive,
  IFramePublisher& publisher,
  ISchedulerClock& clock,
  QObject* parent)
  : QObject(parent)
  , m_configuration(std::move(configuration))
  , m_dataSourceFactory(dataSourceFactory)
  , m_archive(archive)
  , m_publisher(publisher)
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
        m_publisher,
        m_clock,
        m_runtime))
  {
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