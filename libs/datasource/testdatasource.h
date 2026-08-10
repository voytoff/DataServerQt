#pragma once

#include "idatasource.h"
#include <qjsonobject.h>

namespace qds
{

class TestDataSource : public IDataSource
{
public:
  TestDataSource();
  explicit TestDataSource(const QJsonObject& settings);

  bool acquire(
    std::span<double> values) override;

private:
  double m_counter = 0.0;
};

}