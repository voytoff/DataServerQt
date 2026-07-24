#ifndef HARDWAREMODULEFACTORY_H
#define HARDWAREMODULEFACTORY_H

#include "ihardwaremodule.h"
#include "moduleinfo.h"
#include "ihardwaremodulefactory.h"
#include <memory>

namespace qds
{

class HardwareModuleFactory : public IHardwareModuleFactory
{
public:
  std::unique_ptr<IHardwareModule> create(
    const ModuleInfo& info) override;
};

}

#endif // HARDWAREMODULEFACTORY_H
