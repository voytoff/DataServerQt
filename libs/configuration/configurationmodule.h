#pragma once

#include "datatypes.h"

#include <QJsonObject>

namespace qds
{

struct ConfigurationModule
{
  ConfigurationId configurationId = ConfigurationId{0};
  ModuleId module;

  QJsonObject settings;
};

}