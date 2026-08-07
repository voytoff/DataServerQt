#pragma once

#include "idatasource.h"

namespace qds
{

class FailingDataSource : public IDataSource
{
public:

  bool acquire(RawMemory&) override
  {
    return false;
  }
};

}
