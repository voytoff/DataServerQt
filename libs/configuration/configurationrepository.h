#pragma once

#include "systemconfiguration.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariantList>
#include <QVariant>

namespace qds
{

class ConfigurationRepository
{
public:
  explicit ConfigurationRepository(
    const QSqlDatabase& db);

  bool load(
    ConfigurationId id,
    SystemConfiguration& configuration);

private:
  QSqlQuery getQuery(const QString &query, const QVariantMap& args);

private:
  const QSqlDatabase& m_db;

};

}