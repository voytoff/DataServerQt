#pragma once

#include "idatasource.h"
#include <qjsonobject.h>

namespace qds
{

class FailingDataSource : public IDataSource
{
public:
  FailingDataSource(){}
  explicit FailingDataSource(const QJsonObject& settings){}

  bool acquire(std::span<double> values) override
  {
    return false;
  }
};

}
