#include "hardwaremodulefactory.h"
#include "fakelcardmodule.h"
#include "ihardwaremodule.h"
#include "moduleinfo.h"
#include <memory>

namespace qds
{

std::unique_ptr<IHardwareModule> HardwareModuleFactory::create(const ModuleInfo &info)
{
  switch (info.type)
  {
  case ModuleType::Fake:
    return std::make_unique<FakeLCardModule>();

  //case ModuleType::LTR11:
  //  return std::make_unique<LCardModule>(info);

  default:
    return nullptr;
  }
}

}
