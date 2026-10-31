#include "modulevalidators.h"

#include "ltr11configurationbuilder.h"
#include "ltr11configuration.h"

namespace qds
{

bool registerModuleValidators(
  ModuleConfigurationValidatorRegistry& registry)
{
  if (!registry.registerType(
        ModuleType::Test,
        [](const ModuleRuntimeConfiguration&,
           std::string&)
        {
          return true;
        }))
  {
    return false;
  }

  if (!registry.registerType(
        ModuleType::LTR11,
        [](const ModuleRuntimeConfiguration& cfg,
           std::string& error)
        {
          Ltr11Configuration configuration;
          Ltr11ConfigurationBuilder builder;

          if (!builder.build(cfg, configuration))
          {
            error = builder.lastError();
            return false;
          }

          return true;
        }))
  {
    return false;
  }

  return true;
}

}