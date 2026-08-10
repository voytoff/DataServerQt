#pragma once

#include <QJsonObject>
#include "moduleinfo.h"

namespace qds
{

struct ModuleConfiguration
{
  ModuleInfo module;
  QJsonObject settings;
  uint32_t channelCount = 0;
};

}