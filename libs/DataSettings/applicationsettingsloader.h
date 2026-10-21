#pragma once

#include "datasettings_export.h"
#include "applicationsettings.h"

#include <QCoreApplication>
#include <qobject.h>

namespace qds
{

class DATASETTINGS_EXPORT ApplicationSettingsLoader
{
public:
  static bool load(
    const QCoreApplication& app,
    ApplicationSettings& settings,
    QString& error);

  static bool saveConfigurationId(
    const QString& fileName,
    ConfigurationId id,
    const QString& key = "Server/ConfigurationId");
};

}
