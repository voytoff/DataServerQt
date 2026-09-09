#pragma once

#include "configurationmodule.h"
#include "configurationtag.h"
#include "moduleinfo.h"
#include "crateinfo.h"

#include <vector>

namespace qds
{

struct ModuleRuntimeConfiguration
{
  ModuleInfo module;
  CrateInfo crate;

  ConfigurationModule configuration;

  std::vector<ConfigurationTag> tags;
};

}