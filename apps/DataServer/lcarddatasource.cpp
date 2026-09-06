#include "lcarddatasource.h"

#include <algorithm>

namespace qds
{

LCardDataSource::LCardDataSource(
  const ModuleConfiguration& configuration,
  std::unique_ptr<ILCardModule> module,
  std::chrono::microseconds pollInterval)
  : m_configuration(configuration)
  , m_module(std::move(module))
  , m_values(configuration.channelCount, 0.0)
  , m_pollInterval(pollInterval)
{
}

LCardDataSource::~LCardDataSource() noexcept
{
  stop();
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

  if (m_thread.joinable())
    m_thread.join();

  m_module->stop();
}

bool LCardDataSource::acquire(std::span<double> values)
{
  if (!m_running)
    return false;

  if (values.size() != m_values.size())
    return false;

  std::lock_guard lock(m_valuesMutex);

  std::ranges::copy(m_values, values.begin());

  return true;
}

void LCardDataSource::run() noexcept
{
  std::vector<double> work(m_values.size());

  while (m_running)
  {
    if (m_module->read(work))
    {
      std::lock_guard lock(m_valuesMutex);

      std::ranges::copy(work, m_values.begin());
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