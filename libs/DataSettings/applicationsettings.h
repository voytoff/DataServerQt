#pragma once

#include "databasesettings.h"
#include "datatypes.h"
#include "datasettings.h"

#include <QString>

#include <filesystem>

namespace qds
{

struct ApplicationSettings
{
  QString configFile;

  DatabaseSettings database;

  ConfigurationId configurationId{};

  TableSettings table;
  ChartSettings chart;

  std::filesystem::path archiveDirectory;
  std::filesystem::path logDirectory;
};

}