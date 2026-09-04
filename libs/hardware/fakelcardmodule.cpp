#include "fakelcardmodule.h"

namespace qds
{

bool FakeLCardModule::start() noexcept
{
  if (m_running)
    return false;

  ++startCalls;
  m_running = true;

  return true;
}

void FakeLCardModule::stop() noexcept
{
  ++stopCalls;
  m_running = false;
}

bool FakeLCardModule::read(std::span<double> values) noexcept
{
  if (!m_running)
    return false;

  ++readCalls;

  for (std::size_t n = 0; n < values.size(); ++n)
    values[n] = m_counter++;

  return true;
}

}