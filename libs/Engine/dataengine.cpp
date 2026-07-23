#include "dataengine.h"

namespace qds
{

DataEngine::DataEngine(
  DataSourceManager& sources,
  LiveScheduler& scheduler)
  : m_sources(sources)
  , m_scheduler(scheduler)
{
}

bool DataEngine::start()
{
  if (m_running)
    return false;

  if (!m_sources.start())
    return false;

  m_running = true;
  return true;
}

void DataEngine::stop() noexcept
{
  m_sources.stop();
  m_running = false;
}

bool DataEngine::step()
{
  if (!m_running)
    return false;

  if (!m_sources.step())
    return false;

  return m_scheduler.step();
}

bool DataEngine::isRunning() const noexcept
{
  return m_running;
}

}