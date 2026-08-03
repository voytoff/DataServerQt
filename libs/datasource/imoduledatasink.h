#pragma once

#include "datatypes.h"
#include <span>

namespace qds
{

/** куда источник пишет данные */
class IModuleDataSink // TODO -> IDataSink or ISignalInput
{
public:
  virtual ~IModuleDataSink() = default;

  virtual bool updateModule(
    ModuleId module,
    std::span<const float> values,
    uint64_t timestamp) = 0;
};

}
