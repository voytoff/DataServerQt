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

  bool start() noexcept override;

  void stop() noexcept override;

private:
  double m_counter = 0.0;
  bool m_running = false;

};

}