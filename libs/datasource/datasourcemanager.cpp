#include "datasourcemanager.h"

namespace qds
{

bool DataSourceManager::add(std::unique_ptr<IDataSource> source)
{
  if (m_running)
    return false;

  if (!source)
    return false;

  m_sources.push_back(std::move(source));
  return true;

}

bool DataSourceManager::start()
{
  if (m_running)
    return false;

  for (auto& source : m_sources)
  {
    if (!source->start())
    {
      stop();
      return false;
    }
  }

  m_running = true;
  return true;
}

void DataSourceManager::stop() noexcept
{
  for (auto& s : m_sources)
    s->stop();

  m_running = false;
}

bool DataSourceManager::step()
{
  if (!m_running)
    return false;

  for (auto& source : m_sources)
  {
    if (!source->step())
      return false;
  }

  return true;
}

bool DataSourceManager::isRunning() const noexcept
{
  return m_running;
}

std::size_t DataSourceManager::size() const noexcept
{
  return m_sources.size();
}

}