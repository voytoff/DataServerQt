#include "lcarddatasource.h"

#include <algorithm>

namespace qds
{

LCardDataSource::LCardDataSource(
  uint32_t channelCount,
  std::unique_ptr<ILCardModule> module)
  : m_module(std::move(module))
  , m_values(channelCount, 0.0)
  , m_work(channelCount, 0.0)
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

  if (!m_module)
    return false;

  if (!m_module->start())
    return false;

  m_running = true;

  try
  {
    m_thread =
      std::thread(
        &LCardDataSource::run,
        this);
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

  if (m_thread.joinable())
    m_thread.join();

  m_module->stop();
}


bool LCardDataSource::acquire(
  std::span<double> values)
{
  if (!m_running)
    return false;

  if (values.size() != m_values.size())
    return false;

  std::lock_guard lock(m_valuesMutex);

  std::ranges::copy(
    m_values,
    values.begin());

  return true;
}


void LCardDataSource::run() noexcept
{
  while (m_running)
  {
    if (!m_module->read(m_work))
      continue;

    std::lock_guard lock(m_valuesMutex);

    std::ranges::copy(
      m_work,
      m_values.begin());
  }
}

}