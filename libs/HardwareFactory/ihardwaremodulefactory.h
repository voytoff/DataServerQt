#pragma once

#include "ilcardmodule.h"
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