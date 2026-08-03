#pragma once

#include "isignalprocessor.h"

namespace qds
{

class EmptySignalProcessor : public ISignalProcessor
{
public:
  void process(
    const RawMemory&,
    CalculatedMemory&) override
  {
    // ничего не делает
  }
};

}
