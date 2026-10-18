#pragma once

#include "datatypes.h"

#include <QString>
#include <cstdint>

namespace qds
{

struct ConfigurationInfo
{
  ConfigurationId id;

  QString name;
  QString description;

  uint16_t udpPort = 5000;
};

}