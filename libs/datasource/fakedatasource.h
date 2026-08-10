#pragma once

#include "idatasource.h"
#include <QtCore/qjsonobject.h>

namespace qds
{

class FakeDataSource : public IDataSource
{
public:
  FakeDataSource();
  explicit FakeDataSource(const QJsonObject& settings);

  bool acquire(
    std::span<double> values) override;

  const QJsonObject m_settings;

private:
  std::size_t m_size = 1;
  double m_value = 0.0;
};

}
