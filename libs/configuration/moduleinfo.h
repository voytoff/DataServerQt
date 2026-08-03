#pragma once

#include "datatypes.h"
#include "moduletype.h"

namespace qds
{

struct ModuleInfo
{
  ModuleId id;
  CrateId crate;

  ModuleType type = ModuleType::Unknown;
};

}