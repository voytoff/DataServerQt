#pragma once

#include "datatypes.h"
#include "moduletype.h"
#include <qjsonobject.h>

namespace qds
{

struct ModuleInfo
{
  ModuleId id;
  QString serial;
  CrateId crate;

  ModuleType type = ModuleType::Unknown;

  QJsonObject settings;

  QString description;

};

}