#pragma once

#include "datatypes.h"

#include <QJsonObject>

namespace qds
{

struct ConfigurationModule
{
  uint32_t configurationId = 0;
  ModuleId module;

  QJsonObject settings;
};

}