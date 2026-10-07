#pragma once

#include "datatypes.h"
#include "moduletype.h"

#include <QString>
#include <QJsonObject>

namespace qds
{

struct ModuleInfo
{
  ModuleId id;
  QString serial;
  CrateId crate;
  uint32_t slot = 0;
  ModuleType type = ModuleType::Unknown;
  QString description;
};

}