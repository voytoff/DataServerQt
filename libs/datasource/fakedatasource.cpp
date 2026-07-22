#include "fakedatasource.h"

namespace qds
{

bool FakeDataSource::start()
{
  ++startCalls;
  m_running = true;
  return true;
}

void FakeDataSource::stop() noexcept
{
  ++stopCalls;
  m_running = false;
}

bool FakeDataSource::step()
{
  ++stepCalls;
  return m_running;
}

bool FakeDataSource::isRunning() const noexcept
{
  return m_running;
}

}
