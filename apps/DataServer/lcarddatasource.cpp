#include "lcarddatasource.h"

namespace qds
{

LCardDataSource::LCardDataSource(
  const ModuleConfiguration &configuration,
  std::unique_ptr<ILCardModule> module)
  : m_configuration(configuration)
  , m_module(std::move(module))
  , m_values(configuration.channelCount, 0.0)
{
  m_pollInterval = std::chrono::milliseconds(1);
}

bool LCardDataSource::start() noexcept
{
  if (m_running)
    return true;

  if (!m_module->start())
    return false;

  m_running = true;

  try
  {
    m_thread = std::thread(&LCardDataSource::run, this);
  }
  catch (...)
  {
    m_running = false;
    m_module->stop();
    return false;
  }

  return true;
}

void LCardDataSource::stop() noexcept
{
  if (!m_running)
    return;

  m_running = false;

  m_waitCondition.notify_one();

  m_thread.join();

  m_module->stop();
}

bool LCardDataSource::acquire(std::span<double> values)
{
  if (values.size() != m_values.size())
    return false;

  std::lock_guard lock(m_valuesMutex);

  std::ranges::copy(m_values, values.begin());

  return true;
}

void LCardDataSource::run() noexcept
{
  std::vector<double> values(m_values.size());

  while (m_running)
  {
    if (m_module->read(values))
    {
      std::lock_guard lock(m_waitMutex);
      std::ranges::copy(values, m_values.begin());
    }

    std::unique_lock lock(m_waitMutex);

    m_waitCondition.wait_for(
      lock,
      m_pollInterval,
      [this]
      {
        return !m_running;
      });
  }
}

}
