#pragma once

#include "moduleinfo.h"
#include "crateinfo.h"
#include "taginfo.h"

#include <QJsonObject>
#include <vector>

namespace qds
{

struct RuntimeTagConfiguration
{
  TagInfo tag;
  QJsonObject settings;
};

struct RuntimeModuleConfiguration
{
  ModuleInfo module;
  CrateInfo crate;

  QJsonObject settings;

  std::vector<RuntimeTagConfiguration> tags;
};

}