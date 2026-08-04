#pragma once

#include "idatasource.h"

namespace qds
{

class FakeDataSource : public IDataSource
{
public:

  bool acquire(RawMemory& memory) override;
};

}
