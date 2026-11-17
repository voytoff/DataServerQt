#pragma once

#include "datawidgets_export.h"
#include "applicationsettings.h"
#include "settingssection.h"

#include <QApplication>
#include <QSqlDatabase>

namespace qds
{

DATAWIDGETS_EXPORT bool initializeApplication(
  QApplication& app,
  ApplicationSettings& settings,
  QSqlDatabase& database,
  SettingsSections sections);

}