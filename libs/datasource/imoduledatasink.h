#ifndef IMODULEDATASINK_H
#define IMODULEDATASINK_H

#include "datatypes.h"
#include <span>

namespace qds
{

/** куда источник пишет данные */
class IModuleDataSink
{
public:
  virtual ~IModuleDataSink() = default;

  virtual bool updateModule(
    ModuleId module,
    std::span<const float> values,
    uint64_t timestamp) = 0;
};

}

#endif // IMODULEDATASINK_H
