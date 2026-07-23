#include "fakelcardmodule.h"

namespace qds
{

bool FakeLCardModule::start()
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

bool FakeLCardModule::read(std::span<float> values)
{
  if (!m_running)
    return false;

  ++readCalls;
  for (int n = 0; n < values.size(); n++) {
    values[n] = counter++;
  }
  return true;
}

}