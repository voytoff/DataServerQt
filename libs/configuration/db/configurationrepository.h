#pragma once

#include "availablemodule.h"
#include "calibrationrepository.h"
#include "configmodule.h"
#include "datatypes.h"
#include "db/sensorrepository.h"
#include "systemconfiguration.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariantList>
#include <QVariant>
#include <qsqlerror.h>

namespace qds
{

class ConfigurationRepository
{
public:
  explicit ConfigurationRepository(
    const QSqlDatabase& database);

  [[nodiscard]]
  bool load(
    const ConfigurationId &configuration,
    SystemConfiguration& system);

  bool loadSensors(
    SensorRepository& repository);


  [[nodiscard]]
  std::optional<ConfigurationId> addConfiguration(
    const QString& name,
    const QString& description,
    uint16_t udpPort);

  [[nodiscard]]
  std::optional<CrateId> addCrate(
    const CrateType& type,
    const QString& serial,
    const QString& host,
    const uint32_t port,
    const QString& description);

  [[nodiscard]]
  bool crates(
    std::vector<CrateInfo>& crates);

  [[nodiscard]]
  std::optional<ModuleId> addModule(
    const CrateId& crateId,
    const ModuleType& type,
    const QString& serial,
    const uint16_t slot,
    const QString& description);

  [[nodiscard]]
  bool modules(
    std::vector<ModuleInfo>& modules);

  [[nodiscard]]
  bool removeModule(
    const ModuleId &module);

  [[nodiscard]]
  std::optional<TagId> addConfigTag(
    const ConfigurationId& configuration,
    const ModuleId &module,
    const ChannelId& channel,
    const QJsonObject& settings);

  [[nodiscard]]
  bool removeConfigTag(
    const ConfigurationId& configuration,
    const ModuleId &module,
    const ChannelId &channel);

  [[nodiscard]]
  bool moduleConfigTags(
    const ConfigurationId& configuration,
    const ModuleId& module,
    std::vector<ConfigurationTag>& tags);

  [[nodiscard]]
  bool addConfigModule(
    const ConfigurationId& configuration,
    const ModuleId &module);

  [[nodiscard]]
  bool removeConfigModule(
    const ConfigurationId& configuration,
    const ModuleId &module);

  [[nodiscard]]
  bool loadCalibrations(
    const SystemConfiguration& configuration,
    CalibrationRepository& calibrations);

  [[nodiscard]]
  bool configModules(
    const ConfigurationId& configuration,
    std::vector<ConfigModule>& modules);

  [[nodiscard]]
  bool availableModules(
    const ConfigurationId& configuration,
    std::vector<AvailableModule>& modules);

  [[nodiscard]]
  bool updateConfigModuleSettings(
    const ConfigurationId& configuration,
    const ModuleId& module,
    const QJsonObject& settings);

  [[nodiscard]]
  bool updateConfigTagSettings(
    const ConfigurationId& configuration,
    const ModuleId& module,
    const ChannelId& channel,
    const QJsonObject& settings);

  bool setConfigModuleActive(
    const ConfigurationId& configuration,
    const ModuleId& module,
    bool active);

  bool setConfigTagActive(
    const ConfigurationId& configuration,
    const ModuleId& module,
    const ChannelId& channel,
    bool active);

  [[nodiscard]]
  bool signalDefinitions(
    const ConfigurationId& configuration,
    std::vector<SignalDefinition>& definitions);

  [[nodiscard]]
  bool updateSignalName(
    ConfigurationId configuration,
    SignalId signal,
    const QString& name);

  [[nodiscard]]
  bool updateSignalArchiveRate(
    ConfigurationId configuration,
    SignalId signal,
    ArchiveRate rate);

  [[nodiscard]]
  bool updateSignalSensor(
    const ConfigurationId &configuration,
    const SignalId &signal,
    const SensorId &sensor);

  [[nodiscard]]
  bool updateSignalSignalType(
    ConfigurationId configuration,
    SignalId signal,
    SignalTypeId type);

  [[nodiscard]]
  bool updateSignalSource(
    ConfigurationId configuration,
    SignalId signal,
    TagId tag);

  [[nodiscard]]
  bool updateSignalFormula(
    ConfigurationId configuration,
    SignalId signal,
    QString formula);

  [[nodiscard]]
  bool updateSignalPreprocessingFormula(
    ConfigurationId configuration,
    SignalId signal,
    QString formula);

  [[nodiscard]]
  bool createSignal(
    ConfigurationId configuration,
    const QString& name,
    SignalKind kind,
    SignalId& createdId);

  [[nodiscard]]
  bool removeSignal(
    ConfigurationId configuration,
    SignalId signal);

  [[nodiscard]]
  bool signalTypes(
    std::vector<SignalTypeInfo>& types);

  [[nodiscard]]
  bool tagSources(
    ConfigurationId configuration,
    std::vector<TagSourceInfo>& sources);

  [[nodiscard]]
  QSqlError lastError() const;

private:

  QSqlQuery getQuery(
    const QString &sql, const QVariantMap& args = {});

  [[nodiscard]]
  bool exec(
    QSqlQuery& query);

  void setError(
    const QSqlQuery& query);

private:
  const QSqlDatabase& m_database;

  QSqlError m_error;

};

}