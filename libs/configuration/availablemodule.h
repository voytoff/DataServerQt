#pragma once

#include "datatypes.h"
#include "moduletype.h"
#include <QString>

namespace qds
{

struct AvailableModule
{
  ModuleId module;
  QString moduleSerial;
  ModuleType type;

  CrateId crate;
  QString crateSerial;

  uint32_t slot = 0;
};

}