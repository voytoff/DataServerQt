#pragma once

#include "idatasource.h"
#include <qjsonobject.h>
#include <span>

namespace qds
{

class FailOnceDataSource : public IDataSource
{
public:
  FailOnceDataSource();
  explicit FailOnceDataSource(const QJsonObject& settings);

  bool acquire(std::span<double> values) override;

private:

  bool m_fail = true;
};

}
