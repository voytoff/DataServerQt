#include "fakedatasource.h"

namespace qds
{

FakeDataSource::FakeDataSource() { }

FakeDataSource::FakeDataSource(const QJsonObject &settings)
  : m_settings(settings)
{
  if (settings.contains("size"))
    m_size = static_cast<std::size_t>(settings["size"].toInt(0));
}

bool FakeDataSource::acquire(std::span<double> values)
{
  if (m_size > 0)
    Q_ASSERT(values.size() == m_size);

  for (int n = 0; n < values.size(); n++)
    values[n] = ++m_value;

  return true;
}

}