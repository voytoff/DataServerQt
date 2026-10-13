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

  if (!exec(query))
    return false;

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

  // 2. загружаем теги
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

  if (!exec(query))
    return false;

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
SELECT
  id,
  configuration_id,
  name,
  kind,
  tag_id,
  sensor_id,
  signal_type_id,
  formula,
  preprocessing_formula,
  archive_rate
FROM configuration_signal_definition
WHERE configuration_id = :id;)",
    {
      {":id", configuration.value}
    });

  if (!exec(query))
    return false;

  while (query.next())
  {
    SignalDefinition definition;

    definition.id =
      SignalId{query.value("id").toUInt()};

    definition.name =
      query.value("name").toString().toStdString();

    definition.kind =
      static_cast<SignalKind>(
        query.value("kind").toUInt());

    definition.signalType =
      SignalTypeId{
      query.value("signal_type_id").toUInt()
    };

    definition.archiveRate =
      static_cast<ArchiveRate>(
        query.value("archive_rate").toUInt());

    if (!isValidArchiveRate(definition.archiveRate))
      return false;

    if (definition.kind == SignalKind::Raw)
    {
      const TagId tag{
        query.value("tag_id").toUInt()
      };

      if (!cfg.findTag(tag))
        return false;

      definition.source = SignalSource{tag};

      definition.sensorId =
        SensorId{
        query.value("sensor_id").toUInt()
      };

      definition.preprocessingFormula =
        query.value("preprocessing_formula")
          .toString().toStdString();
    }
    else if (definition.kind == SignalKind::Calculated)
    {
      definition.formula =
        query.value("formula").toString().toStdString();
    }
    else
    {
      return false;
    }

    if (!cfg.addSignalDefinition(definition))
      return false;
  }

  system = std::move(cfg);

  return true;
}

bool ConfigurationRepository::loadSensors(
  SensorRepository& repository)
{
  SensorRepository tmp;

  // Типы датчиков.
  {
    auto query = getQuery(R"(
      SELECT
        id,
        name,
        description,
        calibration_kind
      FROM sensor_type
      ORDER BY id;
    )");

    if (!exec(query))
      return false;

    while (query.next())
    {
      SensorType type;

      type.id = SensorTypeId{
        query.value("id").toUInt()
      };

      type.name =
        query.value("name").toString().toStdString();

      type.description =
        query.value("description").toString().toStdString();

      const int kind =
        query.value("calibration_kind").toInt();

      if (kind < 0 || kind > 1)
        return false;

      type.calibrationKind =
        static_cast<CalibrationKind>(kind);

      if (!tmp.add(type))
        return false;
    }

    if (query.lastError().isValid())
    {
      setError(query);
      return false;
    }
  }

  // Датчики.
  {
    auto query = getQuery(R"(
      SELECT
        id,
        sensor_type_id,
        name,
        serial,
        description,
        calibration_mode,
        calibration_kind
      FROM sensor
      ORDER BY id;
    )");

    if (!exec(query))
      return false;

    while (query.next())
    {
      Sensor sensor;

      sensor.id = SensorId{
        query.value("id").toUInt()
      };

      sensor.sensorTypeId = SensorTypeId{
        query.value("sensor_type_id").toUInt()
      };

      sensor.name =
        query.value("name").toString().toStdString();

      sensor.serial =
        query.value("serial").toString().toStdString();

      sensor.description =
        query.value("description").toString().toStdString();

      const int mode =
        query.value("calibration_mode").toInt();

      const int kind =
        query.value("calibration_kind").toInt();

      if (mode < 0 || mode > 1 ||
          kind < 0 || kind > 1)
      {
        return false;
      }

      sensor.calibrationMode =
        static_cast<SensorCalibrationMode>(mode);

      sensor.calibrationKind =
        static_cast<CalibrationKind>(kind);

      // Проверяем существование типа датчика.
      if (!tmp.find(sensor.sensorTypeId))
        return false;

      if (!tmp.add(sensor))
        return false;
    }

    if (query.lastError().isValid())
    {
      setError(query);
      return false;
    }
  }

  // Обновляем репозиторий только после
  // полной успешной загрузки.
  repository = std::move(tmp);

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

  if (!exec(query))
    return false;

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

  return exec(query);
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

  return exec(query);
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

  if (!exec(query))
    return false;

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

bool ConfigurationRepository::loadCalibrations(
  const SystemConfiguration&,
  CalibrationRepository& calibrations)
{
  m_error = {};

  CalibrationRepository repo;

  const QString sql = R"(
SELECT
  id,
  sensor_id,
  sensor_type_id,
  kind,
  name,
  description
FROM calibration
ORDER BY id;)";

  auto query = getQuery(sql, {});

  if (!exec(query))
    return false;

  while (query.next())
  {
    Calibration calibration;

    calibration.id = CalibrationId{query.value("id").toUInt()};
    calibration.sensorId = SensorId{query.value("sensor_id").toUInt()};
    calibration.sensorTypeId = SensorTypeId{query.value("sensor_type_id").toUInt()};
    calibration.kind =
      static_cast<CalibrationKind>(query.value("kind").toInt());

    calibration.name = query.value("name").toString();
    calibration.description = query.value("description").toString();

    // Ровно один владелец.
    if (bool(calibration.sensorId) == bool(calibration.sensorTypeId))
      return false;

    if (calibration.kind != CalibrationKind::Table &&
        calibration.kind != CalibrationKind::Polynomial)
      return false;

    QString dataSql;

    if (calibration.kind == CalibrationKind::Table)
    {
      dataSql = R"(
SELECT `index`, x, y
FROM calibration_point
WHERE calibration_id = :id
ORDER BY `index`;)";
    }
    else
    {
      dataSql = R"(
SELECT `index`, value
FROM calibration_coefficient
WHERE calibration_id = :id
ORDER BY `index`;)";
    }

    auto dataQuery = getQuery(
      dataSql,
      {{":id", calibration.id.value}});

    if (!exec(dataQuery))
      return false;

    if (calibration.kind == CalibrationKind::Table)
    {
      while (dataQuery.next())
      {
        CalibrationPoint point;

        point.index = dataQuery.value("index").toUInt();
        point.x = dataQuery.value("x").toDouble();
        point.y = dataQuery.value("y").toDouble();

        calibration.points.push_back(point);
      }

      if (!calibration.buildSegments())
        return false;
    }
    else
    {
      uint32_t expectedIndex = 0;

      while (dataQuery.next())
      {
        const auto index =
          dataQuery.value("index").toUInt();

        if (index != expectedIndex++)
          return false;

        calibration.coefficients.push_back(
          dataQuery.value("value").toDouble());
      }

      if (calibration.coefficients.empty())
        return false;
    }

    bool added = false;

    if (calibration.sensorId)
    {
      added = repo.addBySensor(
        calibration.sensorId,
        calibration);
    }
    else
    {
      added = repo.addBySensorType(
        calibration.sensorTypeId,
        calibration);
    }

    if (!added)
      return false;
  }

  calibrations = std::move(repo);

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

  if (!exec(query))
    return false;

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

  if (!exec(query))
    return false;

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

  if (!exec(query))
    return false;

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

  return exec(query);
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

  return exec(query);
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

  if (!exec(query))
    return false;

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

  if (!exec(query))
    return false;

  return query.numRowsAffected() == 1;
}

bool ConfigurationRepository::signalDefinitions(
  const ConfigurationId& configuration,
  std::vector<SignalDefinition>& definitions)
{
  m_error = {};

  definitions.clear();

  QSqlQuery query = getQuery(R"(
SELECT
  id,
  name,
  kind,
  tag_id,
  sensor_id,
  signal_type_id,
  archive_rate,
  formula,
  preprocessing_formula
FROM configuration_signal_definition
WHERE configuration_id = :id
ORDER BY id)",
    {
      {":id", configuration.value}
    });

  if (!exec(query))
    return false;

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

    if (!query.value("sensor_id").isNull())
    {
      definition.sensorId = SensorId{
        query.value("sensor_id").toUInt()
      };
    }

    definition.signalType = SignalTypeId{
      query.value("signal_type_id").toUInt()
    };

    definition.archiveRate = static_cast<ArchiveRate>(
      query.value("archive_rate").toInt());

    definition.formula =
      query.value("formula").toString().toStdString();

    definition.preprocessingFormula =
      query.value("preprocessing_formula")
        .toString().toStdString();

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

bool ConfigurationRepository::updateSignalName(
  ConfigurationId configuration,
  SignalId signal,
  const QString& name)
{
  m_error = {};

  auto query = getQuery(R"(
UPDATE configuration_signal_definition
SET name = :name
WHERE configuration_id = :configuration_id
  AND id = :signal_id)",
    {
     {":configuration_id", configuration.value},
     {":signal_id", signal.value},
     {":name", name},
     });

  if (!exec(query))
    return false;

  return query.numRowsAffected() == 1;
}

bool ConfigurationRepository::updateSignalArchiveRate(
  ConfigurationId configuration,
  SignalId signal,
  ArchiveRate rate)
{
  m_error = {};

  if (!isValidArchiveRate(rate))
    return false;

  auto query = getQuery(R"(
UPDATE configuration_signal_definition
SET archive_rate = :archive_rate
WHERE configuration_id = :configuration_id
  AND id = :signal_id)",
    {
     {":configuration_id", configuration.value},
     {":signal_id", signal.value},
     {":archive_rate", static_cast<uint32_t>(rate)},
     });

  if (!exec(query))
    return false;

  return query.numRowsAffected() == 1;
}

bool ConfigurationRepository::updateSignalSensor(
  const ConfigurationId& configuration,
  const SignalId& signal,
  const SensorId& sensor)
{
  m_error = {};

  auto query = getQuery(R"(
UPDATE configuration_signal_definition
SET sensor_id = :sensor_id
WHERE configuration_id = :configuration_id
  AND id = :signal_id
  AND kind = 0;)",
    {
      {":sensor_id",
       sensor ? QVariant(sensor.value) : QVariant()},
      {":configuration_id", configuration.value},
      {":signal_id", signal.value}
    });

  return exec(query);
}

bool ConfigurationRepository::updateSignalSignalType(
  ConfigurationId configuration,
  SignalId signal,
  SignalTypeId type)
{
  m_error = {};

  const QVariant value =
    type.value == 0 ?
      QVariant(QMetaType(QMetaType::Int)) :
      type.value;

  auto query = getQuery(R"(
UPDATE configuration_signal_definition
SET signal_type_id = :signal_type_id
WHERE configuration_id = :configuration_id
  AND id = :signal_id)",
    {
      {":configuration_id", configuration.value},
      {":signal_id", signal.value},
      {":signal_type_id", value}
    });

  if (!exec(query))
    return false;

  return query.numRowsAffected() == 1;
}

bool ConfigurationRepository::updateSignalSource(
  ConfigurationId configuration,
  SignalId signal,
  TagId tag)
{
  m_error = {};

  const QVariant value =
    tag.value == 0
      ? QVariant(QMetaType(QMetaType::Int))
      : QVariant(tag.value);

  auto query = getQuery(R"(
UPDATE configuration_signal_definition
SET tag_id = :tag_id
WHERE configuration_id = :configuration_id
  AND id = :signal_id
  AND kind = 0
  AND (
    :tag_id_check IS NULL
    OR EXISTS (
      SELECT 1
      FROM configuration_tag ct
      WHERE ct.id = :tag_id_source
        AND ct.configuration_id = :configuration_id_source
    )
  ))",
    {
      {":configuration_id", configuration.value},
      {":signal_id", signal.value},
      {":tag_id", value},
      {":tag_id_check", value},
      {":tag_id_source", value},
      {":configuration_id_source", configuration.value}
    });

  if (!exec(query))
    return false;

  return query.numRowsAffected() == 1;
}

bool ConfigurationRepository::updateSignalFormula(
  ConfigurationId configuration,
  SignalId signal,
  QString formula)
{
  m_error = {};

  auto query = getQuery(R"(
UPDATE configuration_signal_definition
SET formula = :formula
WHERE configuration_id = :configuration_id
  AND id = :signal_id)",
    {
     {":configuration_id", configuration.value},
     {":signal_id", signal.value},
     {":formula", formula},
     });

  return exec(query) && query.numRowsAffected() == 1;
}

bool ConfigurationRepository::updateSignalPreprocessingFormula(
  ConfigurationId configuration,
  SignalId signal,
  QString formula)
{
  m_error = {};

  auto query = getQuery(R"(
UPDATE configuration_signal_definition
SET preprocessing_formula = :formula
WHERE configuration_id = :configuration_id
  AND id = :signal_id
  AND kind = 0;)",
    {
      {":configuration_id", configuration.value},
      {":signal_id", signal.value},
      {":formula", formula},
    });

  return exec(query) && query.numRowsAffected() == 1;
}

bool ConfigurationRepository::createSignal(
  ConfigurationId configuration,
  const QString& name,
  SignalKind kind,
  SignalId& createdId)
{
  m_error = {};

  createdId = SignalId{0};

  auto query = getQuery(R"(
INSERT INTO configuration_signal_definition
  (configuration_id, name, kind)
VALUES
  (:configuration_id, :name, :kind))",
    {
      {":configuration_id", configuration.value},
      {":name", name},
      {":kind", static_cast<int>(kind)}
    });

  if (!exec(query))
    return false;

  bool ok = false;

  const uint32_t id =
    query.lastInsertId().toUInt(&ok);

  if (!ok || id == 0)
    return false;

  createdId = SignalId{id};

  return true;
}

bool ConfigurationRepository::removeSignal(
  ConfigurationId configuration,
  SignalId signal)
{
  m_error = {};

  auto query = getQuery(R"(
DELETE FROM configuration_signal_definition
WHERE configuration_id = :configuration_id
  AND id = :signal_id)",
    {
      {":configuration_id", configuration.value},
      {":signal_id", signal.value}
    });

  return exec(query) && query.numRowsAffected() == 1;
}

bool ConfigurationRepository::signalTypes(
  std::vector<SignalTypeInfo>& types)
{
  m_error = {};
  types.clear();

  auto query = getQuery(R"(
SELECT id, name, description
FROM signal_type
ORDER BY name)");

  if (!exec(query))
    return false;

  while (query.next())
  {
    SignalTypeInfo info;

    info.id.value = query.value("id").toUInt();
    info.name = query.value("name").toString().toStdString();
    info.description = query.value("description").toString().toStdString();

    types.push_back(std::move(info));
  }

  return true;
}

bool ConfigurationRepository::tagSources(
  ConfigurationId configuration,
  std::vector<TagSourceInfo>& sources)
{
  m_error = {};
  sources.clear();

  auto query = getQuery(R"(
SELECT ct.id, ct.channel, m.serial
FROM configuration_tag ct
JOIN configuration_module cm
  ON cm.module_id = ct.module_id
 AND cm.configuration_id = ct.configuration_id
JOIN module m
  ON m.id = cm.module_id
WHERE ct.configuration_id = :configuration
ORDER BY m.serial, ct.channel)",
  {
    {":configuration", configuration.value}
  });

  if (!exec(query))
    return false;

  while (query.next())
  {
    TagSourceInfo info;

    info.id.value = query.value("id").toUInt();

    info.name = QString("%1 (%2)")
                  .arg(query.value("channel").toUInt())
                  .arg(query.value("serial").toString())
                  .toStdString();

    sources.push_back(std::move(info));
  }

  return true;
}
/*
void ConfigurationRepository::assignCalibration(Calibration &calibration, const QSqlQuery &query)
{
  calibration.id = CalibrationId{query.value("calibration_id").toUInt()};
  calibration.name = query.value("calibration_name").toString();
  calibration.description = query.value("calibration_description").toString();
  calibration.signalId = SignalId{query.value("signal_id").toUInt()};
  calibration.signalTypeId = SignalTypeId{query.value("signal_type_id").toUInt()};
}
*/
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

  return exec(query);
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

  return exec(query);
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

bool ConfigurationRepository::exec(QSqlQuery &query)
{
  if (!query.exec())
  {
    qDebug() << query.lastError().text();
    qDebug() << query.lastQuery();
    qDebug() << query.boundValues();

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
