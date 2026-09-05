#pragma once

#include "moduleinfo.h"
#include "ihardwaremodule.h"
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
