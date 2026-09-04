#pragma once

#include "ilcardmodule.h"
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
