#include "dataengine.h"

namespace qds
{

bool DataEngine::start() noexcept
{
  if (m_running)
    return false;

  if (!m_sources.start())
    return false;

  m_running = true;
  return true;
}

void DataEngine::stop()
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

}