#include "fakedatasource.h"

namespace qds
{

FakeDataSource::FakeDataSource() { }

FakeDataSource::FakeDataSource(const QJsonObject &settings)
  : m_settings(settings) { }

bool FakeDataSource::acquire(RawMemory &memory)
{
  memory.setValue(0, 123.0);
  return true;
}

}