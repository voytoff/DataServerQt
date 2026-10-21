#pragma once

#include "datasettings_export.h"
#include "databasesettings.h"
#include <QSqlDatabase>

namespace qds
{

class DATASETTINGS_EXPORT DatabaseConnection
{
public:
  bool open(
    const DatabaseSettings& settings,
    QSqlDatabase& database,
    QString& error);
};

}