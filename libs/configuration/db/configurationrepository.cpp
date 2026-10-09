#include <vector>
#include <algorithm>
#include "configurationrepository.h"
#include <QSqlQuery>
#include <QSqlError>

namespace qds
{

ConfigurationRepository::ConfigurationRepository(
  const QSqlDatabase &database)
  : m_database(database) { }

bool ConfigurationRepository::load(
  const ConfigurationId& configuration,
  SystemConfiguration &system)
{
  m_error = {};

  SystemConfiguration cfg;

  // проверяем наличие конфигурации
  auto query = getQuery(R"(
SELECT id, name, description, udp_port
FROM configuration WHERE id = :id;)",
  {
    {":id", configuration.value}
  });

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  if (!query.next())
    return false;

  cfg.setUdpPort(
    query.value("udp_port").toUInt());

  cfg.setName(
    query.value("name").toString().toStdString());

  cfg.setDescription(
    query.value("description").toString().toStdString());

  // 1 загружаем модули
  query = getQuery(R"(
SELECT
  cm.configuration_id, cm.module_id, cm.settings,
  m.type as module_type, m.serial as module_serial, m.slot as module_slot, m.description as module_description,
  c.id as crate_id, c.type as crate_type, c.serial as crate_serial, c.host as crate_host, c.port as crate_port, c.description as crate_description
FROM
  configuration_module cm
JOIN module m
  ON m.id = cm.module_id
JOIN crate c
  ON c.id = m.crate_id
WHERE
  cm.configuration_id = :id
  AND cm.active = 1;)",
  {
    {":id", configuration.value}
  });

  if (!query.exec()) {
    setError(query);
    return false;
  }

  while (query.next())
  {
    ModuleInfo module;
    module.id = ModuleId{query.value("module_id").toUInt()};

    // информация о модуле из module
    module.type = static_cast<ModuleType>(query.value("module_type").toUInt());
    module.crate = CrateId{query.value("crate_id").toUInt()};
    module.serial = query.value("module_serial").toString();
    module.slot = query.value("module_slot").toInt();
    module.description = query.value("module_description").toString();

    const auto& crates = cfg.crates();

    auto it = std::find_if(
      crates.begin(),
      crates.end(),
      [&](const CrateInfo &ci) {
        return ci.id == module.crate;
      });

    if (it == crates.end())
    {
      // запишем информацию о крейте
      CrateInfo crate;
      crate.id = CrateId{query.value("crate_id").toUInt()};
      crate.serial = query.value("crate_serial").toString();
      crate.host = query.value("crate_host").toString();
      crate.port = query.value("crate_port").toUInt();
      crate.description = query.value("crate_description").toString();
      crate.type = static_cast<CrateType>(query.value("crate_type").toUInt());

      cfg.addCrate(crate);
    }

    if (!cfg.addModule(module))
      return false;

    ConfigurationModule configurationModule;

    configurationModule.configurationId = configuration;
    configurationModule.module = module.id;

    const QByteArray data =
      query.value("settings")
        .toString()
        .toUtf8();

    const QJsonDocument document =
      QJsonDocument::fromJson(data);

    if (!document.isObject())
      return false;

    configurationModule.settings =
      document.object();

    if (!cfg.addConfigurationModule(configurationModule))
      return false;
  }

  if (cfg.modules().empty())
    return false;

  // загружаем теги
  query = getQuery(R"(
SELECT
  ct.id,
  ct.configuration_id,
  ct.module_id,
  ct.channel,
  ct.archive_rate,
  ct.settings
FROM configuration_tag ct
JOIN configuration_module cm
  ON cm.configuration_id = ct.configuration_id
  AND cm.module_id = ct.module_id
WHERE
  ct.configuration_id = :id
  AND ct.active = 1
  AND cm.active = 1;)",
  {
    {":id", configuration.value}
  });

  if (!query.exec()) {
    setError(query);
    return false;
  }

  while (query.next())
  {
    TagInfo tag;

    tag.tag = TagId{
      query.value("id").toUInt()
    };

    const ModuleId moduleId{
      query.value("module_id").toUInt()
    };

    const auto* module = cfg.findModule(moduleId);

    if (module == nullptr)
      return false;

    tag.module = module->id;
    tag.channel = ChannelId{
      query.value("channel").toUInt()
    };

    if (!cfg.addTag(tag))
      return false;

    ConfigurationTag configurationTag;
    configurationTag.tag = tag.tag;
    configurationTag.module = tag.module;
    configurationTag.channel = tag.channel;

    configurationTag.archiveRate =
      static_cast<ArchiveRate>(query.value("archive_rate").toUInt());

    if (!isValidArchiveRate(configurationTag.archiveRate))
      return false;

    const QByteArray data =
      query.value("settings")
        .toString()
        .toUtf8();

    const QJsonDocument document =
      QJsonDocument::fromJson(data);

    if (!document.isObject())
      return false;

    configurationTag.settings =
      document.object();

    if (!cfg.addConfigurationTag(configurationTag))
      return false;
  }

  if (cfg.tags().empty())
    return false;

  // загружаем сигналы
  query = getQuery(R"(
SELECT id, configuration_id, name, kind, tag_id,
  signal_type_id, calibration_mode, formula, archive_rate
FROM configuration_signal_definition
WHERE configuration_id = :id;)",
   {
     {":id", configuration.value}
   });

  if (!query.exec()) {
    setError(query);
    return false;
  }

  while (query.next())
  {
    SignalDefinition definition;

    definition.id = SignalId{query.value("id").toUInt()};
    definition.name = query.value("name").toString().toStdString();
    definition.kind = static_cast<SignalKind>(query.value("kind").toUInt());
    definition.signalType = SignalTypeId{query.value("signal_type_id").toUInt()};
    definition.archiveRate = static_cast<ArchiveRate>(query.value("archive_rate").toUInt());

    if (definition.kind == SignalKind::Raw) {

      const TagId tag{
        query.value("tag_id").toUInt()
      };

      if (!cfg.findTag(tag))
        continue;

      definition.source = SignalSource{tag};

    } else if (definition.kind == SignalKind::Calculated) {
      definition.formula = query.value("formula").toString().toStdString();
      definition.calibrationMode = static_cast<CalibrationMode>(query.value("calibration_mode").toUInt());
    } else return false;

    if (!cfg.addSignalDefinition(definition))
      return false;
  }

  system = std::move(cfg);

  return true;
}

std::optional<ConfigurationId> ConfigurationRepository::addConfiguration(
  const QString& name,
  const QString& description,
  uint16_t udpPort)
{
  m_error = {};

  auto query = getQuery(
    R"(
INSERT INTO configuration
  (name, description, udp_port)
VALUES
  (:name, :description, :udp_port);)",
    {
      {":name", name},
      {":description", description},
      {":udp_port", udpPort}
    });

  if (!query.exec()) {
    setError(query);
    return std::nullopt;
  }

  const QVariant value =
    query.lastInsertId();

  if (!value.isValid())
    return std::nullopt;

  return ConfigurationId{
    value.toUInt()
  };
}

std::optional<CrateId> ConfigurationRepository::addCrate(
  const CrateType &type,
  const QString &serial,
  const QString &host,
  const uint32_t port,
  const QString &description)
{
  m_error = {};

  auto query = getQuery(
    R"(
INSERT INTO crate
  (`type`, serial, host, port, description)
VALUES
  (:type, :serial, :host, :port, :description);)",
    {
      {":type", static_cast<uint32_t>(type)},
      {":serial", serial},
      {":host", host},
      {":port", port},
      {":description", description}
    });

  if (!query.exec()) {
    setError(query);
    return std::nullopt;
  }

  const QVariant value =
    query.lastInsertId();

  if (!value.isValid())
    return std::nullopt;

  return CrateId{
    value.toUInt()
  };
}

bool ConfigurationRepository::crates(
  std::vector<CrateInfo> &crates)
{
  m_error = {};

  auto query = getQuery(
    R"(
SELECT id, `type`, serial, host, port, description
FROM crate
ORDER BY id;)");

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  std::vector<CrateInfo> result;

  while (query.next())
  {
    result.push_back({
      .id =
        CrateId{
          query.value("id").toUInt()
        },

      .serial =
        query.value("serial").toString(),

      .type =
        static_cast<CrateType>(query.value("type").toUInt()),

      .host =
        query.value("host").toString(),

      .port =
        query.value("port").toUInt(),

      .description =
        query.value("description").toString(),
    });
  }

  crates = std::move(result);

  return true;
}

std::optional<ModuleId>
ConfigurationRepository::addModule(
  const CrateId &crateId,
  const ModuleType &type,
  const QString& serial,
  const uint16_t slot,
  const QString& description)
{
  m_error = {};

  auto query = getQuery(
    R"(
INSERT INTO module
  (crate_id, type, serial, slot, description)
VALUES
  (:crate_id, :type, :serial, :slot, :description);)",
    {
      {":crate_id", crateId.value},
      {":type", static_cast<uint32_t>(type)},
      {":serial", serial},
      {":slot", slot},
      {":description", description}
    });

  if (!query.exec()) {
    setError(query);
    return std::nullopt;
  }

  const QVariant value =
    query.lastInsertId();

  if (!value.isValid())
    return std::nullopt;

  return ModuleId{
    value.toUInt()
  };
}

bool ConfigurationRepository::modules(
  std::vector<ModuleInfo> &modules)
{
  m_error = {};

  auto query = getQuery(
    R"(
SELECT id, crate_id, `type`, serial, slot, description
FROM module
ORDER BY crate_id, slot;)");

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  std::vector<ModuleInfo> result;

  while (query.next())
  {
    result.push_back({
      .id =
        ModuleId{
          query.value("id").toUInt()
        },

      .serial =
        query.value("serial").toString(),

      .crate =
        CrateId{
          query.value("crate_id").toUInt()
        },

      .slot =
        query.value("slot").toUInt(),

      .type =
        static_cast<ModuleType>(query.value("type").toUInt()),

      .description =
        query.value("description").toString(),
    });
  }

  modules = std::move(result);

  return true;
}

bool ConfigurationRepository::removeModule(
  const ModuleId &module)
{
  m_error = {};

  auto query = getQuery(
    R"(
DELETE FROM module
WHERE id = :id;)",
    {
      {":id", module.value}
    });

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  return true;
}

std::optional<TagId> ConfigurationRepository::addConfigTag(
  const ConfigurationId &configuration,
  const ModuleId &module,
  const ChannelId &channel,
  const QJsonObject &settings,
  ArchiveRate archiveRate)
{
  m_error = {};

  const QString json =
    QString::fromUtf8(
      QJsonDocument(settings)
        .toJson(QJsonDocument::Compact));

  auto query = getQuery(
    R"(
INSERT INTO configuration_tag
  (configuration_id, module_id, channel, archive_rate, settings)
VALUES
  (:configuration_id, :module_id, :channel, :archive_rate, :settings);)",
    {
      {":configuration_id", configuration.value},
      {":module_id", module.value},
      {":channel", channel.value},
      {":archive_rate", static_cast<uint16_t>(archiveRate)},
      {":settings", json}
    });

  if (!query.exec()) {
    setError(query);
    return std::nullopt;
  }

  const QVariant value =
    query.lastInsertId();

  if (!value.isValid())
    return std::nullopt;

  return TagId{
    value.toUInt()
  };
}

bool ConfigurationRepository::removeConfigTag(
  const ConfigurationId &configuration,
  const ModuleId &module,
  const ChannelId& channel)
{
  m_error = {};

  auto query = getQuery(
    R"(
DELETE FROM configuration_tag
WHERE configuration_id = :configuration_id
AND module_id = :module_id
AND channel = :channel;)",
    {
      {":configuration_id", configuration.value},
      {":module_id", module.value},
      {":channel", channel.value}
    });

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  return true;
}

bool ConfigurationRepository::moduleConfigTags(
  const ConfigurationId &configuration,
  const ModuleId &module,
  std::vector<ConfigurationTag> &tags)
{
  m_error = {};

  auto query = getQuery(
    R"(
SELECT id, module_id, channel, archive_rate, settings, active
FROM configuration_tag
WHERE configuration_id = :configuration_id
AND module_id = :module_id;)",
    {
      {":configuration_id", configuration.value},
      {":module_id", module.value}
    });

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  std::vector<ConfigurationTag> result;

  while (query.next())
  {
    QJsonParseError error;

    const auto document =
      QJsonDocument::fromJson(
        query.value("settings").toString().toUtf8(),
        &error);

    if (error.error != QJsonParseError::NoError ||
        !document.isObject())
    {
      return false;
    }

    result.push_back({
      .tag =
        TagId{
          query.value("id").toUInt()
        },

      .module =
        ModuleId{
          query.value("module_id").toUInt()
        },

      .channel =
        ChannelId{
          query.value("channel").toUInt()
        },

      .archiveRate =
        static_cast<ArchiveRate>(
          query.value("archive_rate").toUInt()),

      .active =
        query.value("active").toBool(),

      .settings =
        document.object(),
    });
  }

  tags = std::move(result);

  return true;
}

bool ConfigurationRepository::loadCalibrations(const SystemConfiguration &configuration, CalibrationRepository &calibrations)
{
  m_error = {};

  CalibrationRepository repo;
  std::vector<SignalId> signalIds;
  std::vector<SignalTypeId> signalTypes;
  for (auto &definition : configuration.signalDefinitions())
  {
    if (definition.kind == SignalKind::Calculated)
    {
      switch (definition.calibrationMode) {
      case CalibrationMode::BySignal:
        if (std::ranges::find(signalIds, definition.id) == signalIds.end())
          signalIds.push_back(definition.id);
        break;
      case CalibrationMode::BySignalType:
        if (std::ranges::find(signalTypes, definition.signalType) == signalTypes.end())
          signalTypes.push_back(definition.signalType);
        break;
      default:
        break;
      }
    }
  }

  QString sql = R"(
SELECT
  calibration_id, `index`, x, y,
  signal_id, signal_type_id, c.name as calibration_name, c.description as calibration_description
FROM
  calibration_point cp
JOIN calibration c on
  cp.calibration_id = c.id
WHERE
  c.signal_id = :id;)";

  for (const auto &id : signalIds)
  {
    auto query = getQuery(sql, {{":id", id.value}});

    if (!query.exec()) {
      setError(query);
      return false;
    }

    Calibration calibration;
    bool assigned = false;

    while (query.next())
    {
      if (!assigned)
      {
        assignCalibration(calibration, query);
        assigned = true;
      }

      CalibrationPoint point;

      point.index = query.value("index").toUInt();
      point.x = query.value("x").toDouble();
      point.y = query.value("y").toDouble();

      calibration.points.push_back(point);
    }

    if (calibration.points.empty())
      return false;

    if (!calibration.buildSegments())
      return false;

    if (!repo.addBySignal(id, calibration))
      return false;
  }

  sql = R"(
SELECT
  calibration_id, `index`, x, y,
  signal_id, signal_type_id, c.name as calibration_name, c.description as calibration_description
FROM
  calibration_point cp
JOIN calibration c on
  cp.calibration_id = c.id
WHERE
  c.signal_type_id = :id;)";

  for (const auto &id : signalTypes)
  {
    auto query = getQuery(sql, {{":id", id.value}});

    if (!query.exec())
    {
      setError(query);
      return false;
    }

    Calibration calibration;
    bool assigned = false;

    while (query.next())
    {
      if (!assigned)
      {
        assignCalibration(calibration, query);
        assigned = true;
      }

      CalibrationPoint point;

      point.index = query.value("index").toUInt();
      point.x = query.value("x").toDouble();
      point.y = query.value("y").toDouble();

      calibration.points.push_back(point);
    }

    if (calibration.points.empty())
      return false;

    if (!calibration.buildSegments())
      return false;

    if (!repo.addBySignalType(id, calibration))
      return false;
  }

  calibrations = repo;

  return true;
}

bool ConfigurationRepository::configModules(
  const ConfigurationId &configuration,
  std::vector<ConfigModule> &modules)
{
  m_error = {};

  auto query = getQuery(
    R"(
SELECT
  m.id AS module_id,
  m.serial AS module_serial,
  m.type AS module_type,
  m.slot AS module_slot,
  c.id AS crate_id,
  c.serial AS crate_serial,
  cm.settings, cm.active
FROM module m
JOIN crate c
  ON c.id = m.crate_id
JOIN configuration_module cm
  ON cm.module_id = m.id
WHERE
  cm.configuration_id = :configuration_id
ORDER BY
  c.id,
  m.slot;)",
    {
      {":configuration_id", configuration.value}
    });

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  std::vector<ConfigModule> result;

  while (query.next())
  {
    QJsonParseError error;

    const auto document =
      QJsonDocument::fromJson(
        query.value("settings").toString().toUtf8(),
        &error);

    if (error.error != QJsonParseError::NoError ||
        !document.isObject())
    {
      return false;
    }

    result.push_back({
      .module =
        ModuleId{
          query.value("module_id").toUInt()
        },

      .moduleSerial =
      query.value("module_serial").toString(),

      .type =
      static_cast<ModuleType>(
        query.value("module_type").toUInt()),

      .crate =
        CrateId{
          query.value("crate_id").toUInt()
        },

      .crateSerial =
        query.value("crate_serial").toString(),

      .slot =
        query.value("module_slot").toUInt(),

      .active =
        query.value("active").toBool(),

      .settings =
        document.object(),
    });
  }

  modules = std::move(result);

  return true;
}

bool ConfigurationRepository::availableModules(
  const ConfigurationId& configuration,
  std::vector<AvailableModule>& modules)
{
  m_error = {};

  auto query = getQuery(
    R"(
SELECT
  m.id AS module_id,
  m.serial AS module_serial,
  m.type AS module_type,
  m.slot AS module_slot,
  c.id AS crate_id,
  c.serial AS crate_serial
FROM module m
JOIN crate c
  ON c.id = m.crate_id
WHERE NOT EXISTS
(
  SELECT 1
  FROM configuration_module cm
  WHERE cm.configuration_id = :configuration_id
    AND cm.module_id = m.id
)
ORDER BY
  c.id,
  m.slot;)",
    {
      {":configuration_id", configuration.value}
    });

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  std::vector<AvailableModule> result;

  while (query.next())
  {
    result.push_back({
      .module =
        ModuleId{
          query.value("module_id").toUInt()
        },

      .moduleSerial =
        query.value("module_serial").toString(),

      .type =
        static_cast<ModuleType>(
          query.value("module_type").toUInt()),

      .crate =
        CrateId{
          query.value("crate_id").toUInt()
        },

      .crateSerial =
        query.value("crate_serial").toString(),

      .slot =
        query.value("module_slot").toUInt()
    });
  }

  modules = std::move(result);

  return true;
}

bool ConfigurationRepository::updateConfigModuleSettings(
  const ConfigurationId &configuration,
  const ModuleId &module,
  const QJsonObject &settings)
{
  m_error = {};

  const QString json =
    QString::fromUtf8(
      QJsonDocument(settings)
        .toJson(QJsonDocument::Compact));

  auto query = getQuery(
    R"(
UPDATE configuration_module
SET settings = :settings
WHERE configuration_id = :configuration_id
AND module_id = :module_id;)",
    {
      {":configuration_id", configuration.value},
      {":module_id", module.value},
      {":settings", json}
    });

  if (!query.exec())
  {
    setError(query);
    return false;
  }
  /*
  if (query.numRowsAffected() == 0)
  {
    m_error = QSqlError(
      {},
      QStringLiteral("Запись конфигурации не найдена"),
      QSqlError::UnknownError);

    return false;
  }
  */
  return true;
}

bool ConfigurationRepository::updateConfigTagSettings(
  const ConfigurationId &configuration,
  const ModuleId &module,
  const ChannelId &channel,
  const QJsonObject &settings)
{
  m_error = {};

  const QString json =
    QString::fromUtf8(
      QJsonDocument(settings)
        .toJson(QJsonDocument::Compact));

  auto query = getQuery(
    R"(
UPDATE configuration_tag
SET settings = :settings
WHERE configuration_id = :configuration_id
AND module_id = :module_id
AND channel = :channel;)",
    {
      {":configuration_id", configuration.value},
      {":module_id", module.value},
      {":channel", channel.value},
      {":settings", json}
    });

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  return true;
}

bool ConfigurationRepository::updateConfigTagArchiveRate(
  const ConfigurationId& configuration,
  const ModuleId& module,
  const ChannelId& channel,
  ArchiveRate rate)
{
  m_error = {};

  if (!isValidArchiveRate(rate))
    return false;

  auto query = getQuery(
    R"(
UPDATE configuration_tag
SET archive_rate = :archive_rate
WHERE configuration_id = :configuration_id
AND module_id = :module_id
AND channel = :channel;)",
    {
      {":configuration_id", configuration.value},
      {":module_id", module.value},
      {":channel", channel.value},
      {":archive_rate", static_cast<uint16_t>(rate)}
    });

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  return true;
}

bool ConfigurationRepository::setConfigModuleActive(
  const ConfigurationId& configuration,
  const ModuleId& module,
  bool active)
{
  m_error = {};

  auto query = getQuery(R"(
UPDATE configuration_module
SET active = :active
WHERE configuration_id = :configuration_id
  AND module_id = :module_id;)",
  {
    {":configuration_id", configuration.value},
    {":module_id", module.value},
    {":active", active}
  });

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  return query.numRowsAffected() == 1;
}

bool ConfigurationRepository::setConfigTagActive(
  const ConfigurationId& configuration,
  const ModuleId& module,
  const ChannelId& channel,
  bool active)
{
  m_error = {};

  auto query = getQuery(R"(
UPDATE configuration_tag
SET active = :active
WHERE configuration_id = :configuration_id
  AND module_id = :module_id
  AND channel = :channel;)",
  {
    {":configuration_id", configuration.value},
    {":module_id", module.value},
    {":channel", channel.value},
    {":active", active}
  });

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  return query.numRowsAffected() == 1;
}

bool ConfigurationRepository::signalDefinitions(
  const ConfigurationId& configuration,
  std::vector<SignalDefinition>& definitions)
{
  definitions.clear();

  QSqlQuery query = getQuery(
    R"(
      SELECT
        id,
        name,
        kind,
        tag_id,
        signal_type_id,
        archive_rate,
        calibration_mode,
        formula
      FROM configuration_signal_definition
      WHERE configuration_id = :id
      ORDER BY id
    )",
    {
      {":id", configuration.value}
    });

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  while (query.next())
  {
    SignalDefinition definition;

    definition.id = SignalId{
      query.value("id").toUInt()
    };

    definition.name =
      query.value("name").toString().toStdString();

    definition.kind = static_cast<SignalKind>(
      query.value("kind").toInt());

    if (!query.value("tag_id").isNull())
    {
      definition.source.tag = TagId{
        query.value("tag_id").toUInt()
      };
    }

    definition.signalType = SignalTypeId{
      query.value("signal_type_id").toUInt()
    };

    definition.archiveRate = static_cast<ArchiveRate>(
      query.value("archive_rate").toInt());

    definition.calibrationMode = static_cast<CalibrationMode>(
      query.value("calibration_mode").toInt());

    definition.formula =
      query.value("formula").toString().toStdString();

    definitions.push_back(std::move(definition));
  }

  if (query.lastError().isValid())
  {
    setError(query);
    definitions.clear();
    return false;
  }

  return true;
}

QSqlQuery ConfigurationRepository::getQuery(
  const QString& sql,
  const QVariantMap& args)
{
  QSqlQuery query(m_database);

  if (!query.prepare(sql))
    return query;

  for (auto it = args.cbegin();
       it != args.cend();
       ++it)
  {
    query.bindValue(
      it.key(),
      it.value());
  }

  return query;
}

void ConfigurationRepository::assignCalibration(Calibration &calibration, const QSqlQuery &query)
{
  calibration.id = CalibrationId{query.value("calibration_id").toUInt()};
  calibration.name = query.value("calibration_name").toString();
  calibration.description = query.value("calibration_description").toString();
  calibration.signalId = SignalId{query.value("signal_id").toUInt()};
  calibration.signalTypeId = SignalTypeId{query.value("signal_type_id").toUInt()};
}

void ConfigurationRepository::setError(const QSqlQuery &query)
{
  m_error = query.lastError();
  qDebug() << m_error.text();
}

bool ConfigurationRepository::addConfigModule(
  const ConfigurationId& configuration,
  const ModuleId& module)
{
  m_error = {};

  auto query = getQuery(
    R"(
INSERT INTO configuration_module
(
  configuration_id,
  module_id,
  settings
)
VALUES
(
  :configuration_id,
  :module_id,
  '{}'
);)",
    {
      {":configuration_id", configuration.value},
      {":module_id", module.value}
    });

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  return true;
}

bool ConfigurationRepository::removeConfigModule(
  const ConfigurationId &configuration,
  const ModuleId& module)
{
  m_error = {};

  auto query = getQuery(
    R"(
DELETE FROM configuration_module
WHERE configuration_id = :configuration_id
AND module_id = :module_id;)",
    {
      {":configuration_id", configuration.value},
      {":module_id", module.value}
    });

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  return true;
}

QSqlError ConfigurationRepository::lastError() const
{
  return m_error;
}

}
