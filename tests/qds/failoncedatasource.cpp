#include "failoncedatasource.h"

namespace qds
{

FailOnceDataSource::FailOnceDataSource() { }

FailOnceDataSource::FailOnceDataSource(const QJsonObject &settings) { }

bool FailOnceDataSource::acquire(std::span<double> values)

{
  if (m_fail)
  {
    m_fail = false;
    return false;
  }

  for (double& value : values)
    value = 42.0;

  return true;
}

bool FailOnceDataSource::start() noexcept
{
  return true;
}

void FailOnceDataSource::stop() noexcept
{

}

}
