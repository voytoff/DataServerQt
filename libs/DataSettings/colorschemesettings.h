#pragma once

#include "datasettings_export.h"

#include <QString>

namespace qds
{

class DATASETTINGS_EXPORT ColorSchemeSettings
{
public:
  static void restore(
    const QString& configFile);

  static void save(
    const QString& configFile);
};

}