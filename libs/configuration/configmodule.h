#pragma once

#include "datatypes.h"
#include "moduletype.h"
#include <QString>
#include <QJsonObject>

namespace qds
{

struct ConfigModule
{
  ModuleId module;
  QString moduleSerial;
  ModuleType type;

  CrateId crate;
  QString crateSerial;

  uint32_t slot = 0;

  bool active = true;

  QJsonObject settings;
};

}
