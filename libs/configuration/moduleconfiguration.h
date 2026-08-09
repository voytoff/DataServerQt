#pragma once

#include <QJsonObject>
#include "moduleinfo.h"

namespace qds
{

struct ModuleConfiguration
{
  ModuleInfo module;
  QJsonObject settings;
};

}