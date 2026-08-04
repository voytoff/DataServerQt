#pragma once

#include "signalmemory.h"

namespace qds
{

class IDataSource
{
public:
  virtual ~IDataSource() noexcept = default;

  [[nodiscard]]
  virtual bool acquire(RawMemory& memory) = 0;
};

}