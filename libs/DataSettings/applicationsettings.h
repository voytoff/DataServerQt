#pragma once

#include "databasesettings.h"
#include "datatypes.h"

#include <QString>

#include <filesystem>

namespace qds
{

struct ApplicationSettings
{
  QString configFile;

  DatabaseSettings database;

  ConfigurationId configurationId{};

  std::filesystem::path archiveDirectory;
  std::filesystem::path logDirectory;
};

}