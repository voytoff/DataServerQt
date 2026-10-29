#pragma once

#include "datasettings_export.h"
#include "applicationsettings.h"
#include "settingssection.h"

#include <QCoreApplication>

namespace qds
{

class DATASETTINGS_EXPORT ApplicationSettingsLoader
{
public:
  // Загрузка с определением INI через --config.
  static bool load(
    const QCoreApplication& app,
    ApplicationSettings& settings,
    QString& error,
    bool requireServerSettings = false);

  // Загрузка выбранных разделов из конкретного INI.
  static bool load(
    const QString& fileName,
    ApplicationSettings& settings,
    QString& error,
    SettingsSections sections);

  // Сохранение выбранных разделов в INI.
  static bool save(
    const QString& fileName,
    const ApplicationSettings& settings,
    QString& error,
    SettingsSections sections);

  // Сохранение выбранного эксперимента.
  static bool saveConfigurationId(
    const QString& fileName,
    ConfigurationId id,
    const QString& key = "Server/ConfigurationId");
};

}