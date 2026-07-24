#ifndef IHARDWAREMODULEFACTORY_H
#define IHARDWAREMODULEFACTORY_H

#include "ihardwaremodule.h"
#include "moduleinfo.h"
#include <memory>

namespace qds
{

class IHardwareModuleFactory
{
public:
  virtual ~IHardwareModuleFactory() = default;

  [[nodiscard]]
  virtual std::unique_ptr<IHardwareModule> create(
    const ModuleInfo& info) = 0;
};

}

#endif // IHARDWAREMODULEFACTORY_H
