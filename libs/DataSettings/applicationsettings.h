#pragma once

#include "databasesettings.h"
#include "datatypes.h"
#include "tablesettings.h"

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

  std::filesystem::path archiveDirectory;
  std::filesystem::path logDirectory;
};

}