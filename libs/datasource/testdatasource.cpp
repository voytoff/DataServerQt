#include "testdatasource.h"

namespace qds
{

TestDataSource::TestDataSource() { }

TestDataSource::TestDataSource(const QJsonObject &settings) { }

bool TestDataSource::acquire(std::span<double> values)
{
  values[0] = m_counter;
  values[1] = m_counter * 10;

  ++m_counter;

  return true;
}

}
