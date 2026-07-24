#ifndef MODULEINFO_H
#define MODULEINFO_H

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
#endif // MODULEINFO_H
