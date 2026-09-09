#pragma once

#include "datatypes.h"

#include <QJsonObject>

namespace qds
{

struct ConfigurationTag
{
  TagId tag;

  ModuleId module;
  ChannelId channel;

  QJsonObject settings;
};

}