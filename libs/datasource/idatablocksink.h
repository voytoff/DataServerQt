#pragma once

#include "datatypes.h"
#include <span>

namespace qds
{

class IDataBlockSink
{
public:
  virtual ~IDataBlockSink() noexcept = default;

  virtual void push(
    ModuleId module,
    std::span<const double> values,
    std::size_t channelCount,
    std::size_t frameCount) = 0;
};

}