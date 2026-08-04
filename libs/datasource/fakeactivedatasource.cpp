#include "fakeactivedatasource.h"

namespace qds
{

bool FakeActiveDataSource::start()
{
  ++startCalls;
  m_running = true;
  return true;
}

void FakeActiveDataSource::stop() noexcept
{
  ++stopCalls;
  m_running = false;
}

bool FakeActiveDataSource::step() noexcept
{
  ++stepCalls;
  return m_running;
}

bool FakeActiveDataSource::isRunning() const noexcept
{
  return m_running;
}

}
