#pragma once

#include "calibrationrepository.h"
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

bool loadCalibrations(
    const SystemConfiguration& configuration,
    CalibrationRepository& calibrations);

private:
  QSqlQuery getQuery(const QString &sql, const QVariantMap& args);
  void assignCalibration(Calibration &calibration, const QSqlQuery &query);

private:
  const QSqlDatabase& m_db;

};

}