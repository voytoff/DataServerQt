#include "testdatasource.h"

namespace qds
{

TestDataSource::TestDataSource() { }

TestDataSource::TestDataSource(const QJsonObject &settings) { }

bool TestDataSource::start() noexcept
{
  if (m_running)
    return true;

  m_running = true;
  return true;
}

void TestDataSource::stop() noexcept
{
  m_running = false;
}

bool TestDataSource::acquire(std::span<double> values)
{
  if (!m_running)
    return false;

  values[0] = m_counter;
  values[1] = m_counter * 10;

  ++m_counter;

  return true;
}

}
