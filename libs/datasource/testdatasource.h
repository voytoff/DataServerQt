#pragma once

#include "idatasource.h"

namespace qds
{

class TestDataSource : public IDataSource
{
public:
  bool acquire(RawMemory& memory) override;

private:
  double m_counter = 0.0;
};

}