#include "configurationrepository.h"

#include <vector>
#include <algorithm>
#include <QSqlQuery>
#include <QSqlError>
#include <cmath>
#include <utility>
#include <algorithm>
#include <ranges>


namespace qds
{

ConfigurationRepository::ConfigurationRepository(
  const QSqlDatabase &database)
  : m_database(database) { }


QSqlQuery ConfigurationRepository::getQuery(
  const QString& sql,
  const QVariantMap& args)
{
  m_error = {};

  QSqlQuery query(m_database);

  if (!query.prepare(sql))
  {
    setError(query);
    return query;
  }

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

bool ConfigurationRepository::exec(QSqlQuery& query)
{
  if (m_error.isValid())
    return false;

  if (!query.exec())
  {
    setError(query);
    return false;
  }

  return true;
}

void ConfigurationRepository::setError(const QSqlQuery &query)
{
  m_error = query.lastError();
  qDebug() << query.lastError().text();
  qDebug() << query.lastQuery();
  qDebug() << query.boundValues();
}

QSqlError ConfigurationRepository::lastError() const
{
  return m_error;
}

bool ConfigurationRepository::load(
  const ConfigurationId& configuration,
  SystemConfiguration &system)
{
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
      const QVariant tagValue =
        query.value("tag_id");

      if (tagValue.isNull())
        return false;

      const TagId tag{
        tagValue.toUInt()
      };

      if (!cfg.findTag(tag))
        return false;

      definition.source = SignalSource{tag};
    }
    else if (definition.kind == SignalKind::Calculated)
    {
      definition.formula =
        query.value("formula").toString().toStdString();

      const QVariant sensorValue =
        query.value("sensor_id");

      if (!sensorValue.isNull())
      {
        definition.sensorId =
          SensorId{sensorValue.toUInt()};
      }
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
  const ConfigurationInfo& configuration)
{
  auto query = getQuery(R"(
INSERT INTO configuration
  (name, description, udp_port)
VALUES
  (:name, :description, :udp_port);)",
    {
      {":name", configuration.name},
      {":description", configuration.description},
      {":udp_port", configuration.udpPort}
    });

  if (!exec(query))
    return std::nullopt;

  const uint32_t id =
    query.lastInsertId().toUInt();

  if (id == 0)
    return std::nullopt;

  return ConfigurationId{id};
}

bool ConfigurationRepository::updateConfiguration(
  const ConfigurationInfo& configuration)
{
  if (!configuration.id)
    return false;

  auto query = getQuery(R"(
UPDATE configuration
SET
  name = :name,
  description = :description,
  udp_port = :udp_port
WHERE id = :id;)",
    {
      {":id", configuration.id.value},
      {":name", configuration.name},
      {":description", configuration.description},
      {":udp_port", configuration.udpPort}
    });

  if (!exec(query))
    return false;

  return query.numRowsAffected() == 1;
}

bool ConfigurationRepository::removeConfiguration(
  ConfigurationId id)
{
  if (!id)
    return false;

  auto query = getQuery(R"(
DELETE FROM configuration
WHERE id = :id;)",
    {
      {":id", id.value}
    });

  if (!exec(query))
    return false;

  return query.numRowsAffected() == 1;
}

bool ConfigurationRepository::configuration(
  ConfigurationId id,
  ConfigurationInfo& configuration)
{
  if (!id)
    return false;

  auto query = getQuery(R"(
SELECT
  id,
  name,
  description,
  udp_port
FROM configuration
WHERE id = :id;)",
    {
      {":id", id.value}
    });

  if (!exec(query))
    return false;

  if (!query.next())
    return false;

  ConfigurationInfo result;

  result.id = ConfigurationId{
    query.value("id").toUInt()
  };

  result.name =
    query.value("name").toString();

  result.description =
    query.value("description").toString();

  result.udpPort =
    static_cast<uint16_t>(
      query.value("udp_port").toUInt());

  configuration = std::move(result);

  return true;
}

std::optional<CrateId> ConfigurationRepository::addCrate(
  const CrateType &type,
  const QString &serial,
  const QString &host,
  const uint32_t port,
  const QString &description)
{
  auto query = getQuery(R"(
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
  auto query = getQuery(R"(
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
  auto query = getQuery(R"(
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
  auto query = getQuery(R"(
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
  auto query = getQuery(R"(
DELETE FROM module
WHERE id = :id;)",
    {
      {":id", module.value}
    });

  return exec(query);
}

std::optional<TagId> ConfigurationRepository::addConfigTag(
  const ConfigurationId& configuration,
  const ModuleId& module,
  const ChannelId& channel,
  const QJsonObject& settings)
{
  const QString json =
    QString::fromUtf8(
      QJsonDocument(settings)
        .toJson(QJsonDocument::Compact));

  auto query = getQuery(R"(
INSERT INTO configuration_tag
  (configuration_id, module_id, channel, settings)
VALUES
  (:configuration_id, :module_id, :channel, :settings);)",
    {
      {":configuration_id", configuration.value},
      {":module_id", module.value},
      {":channel", channel.value},
      {":settings", json}
    });

  if (!query.exec())
  {
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

bool ConfigurationRepository::moduleConfigTags(
  const ConfigurationId &configuration,
  const ModuleId &module,
  std::vector<ConfigurationTag> &tags)
{
  auto query = getQuery(R"(
SELECT id, module_id, channel, settings, active
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
  CalibrationRepository repo;

  auto query = getQuery(R"(
SELECT
  id,
  sensor_id,
  sensor_type_id,
  kind,
  description
FROM calibration
ORDER BY id;)");

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

    if (dataQuery.lastError().isValid())
    {
      setError(dataQuery);
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

  if (query.lastError().isValid())
  {
    setError(query);
    return false;
  }

  calibrations = std::move(repo);

  return true;
}

bool ConfigurationRepository::configModules(
  const ConfigurationId &configuration,
  std::vector<ConfigModule> &modules)
{
  auto query = getQuery(R"(
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
  auto query = getQuery(R"(
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
  const QString json =
    QString::fromUtf8(
      QJsonDocument(settings)
        .toJson(QJsonDocument::Compact));

  auto query = getQuery(R"(
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
  const QString json =
    QString::fromUtf8(
      QJsonDocument(settings)
        .toJson(QJsonDocument::Compact));

  auto query = getQuery(R"(
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

bool ConfigurationRepository::setConfigModuleActive(
  const ConfigurationId& configuration,
  const ModuleId& module,
  bool active)
{
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
  formula
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

    if (definition.kind == SignalKind::Raw)
    {
      if (!query.value("tag_id").isNull())
      {
        definition.source.tag = TagId{
          query.value("tag_id").toUInt()
        };
      }
    }    else if (definition.kind == SignalKind::Calculated)
    {
      if (!query.value("sensor_id").isNull())
      {
        definition.sensorId = SensorId{
          query.value("sensor_id").toUInt()
        };
      }
    }
    else
    {
      definitions.clear();
      return false;
    }

    definition.signalType = SignalTypeId{
      query.value("signal_type_id").toUInt()
    };

    definition.archiveRate = static_cast<ArchiveRate>(
      query.value("archive_rate").toInt());

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

bool ConfigurationRepository::updateSignalName(
  ConfigurationId configuration,
  SignalId signal,
  const QString& name)
{
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
  auto query = getQuery(R"(
UPDATE configuration_signal_definition
SET sensor_id = :sensor_id
WHERE configuration_id = :configuration_id
  AND id = :signal_id
  AND kind = 1;)",
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
  AND ct.active = 1
  AND cm.active = 1
ORDER BY m.serial, ct.channel)",
    {
      {":configuration", configuration.value}
    });

  if (!exec(query))
    return false;

  while (query.next())
  {
    TagSourceInfo info;

    info.id.value =
      query.value("id").toUInt();

    info.name =
      QString("%1 (%2)")
        .arg(query.value("channel").toUInt() + 1)
        .arg(query.value("serial").toString())
        .toStdString();

    sources.push_back(std::move(info));
  }

  return true;
}

bool ConfigurationRepository::addConfigModule(
  const ConfigurationId& configuration,
  const ModuleId& module)
{
  auto query = getQuery(
    R"(
INSERT INTO configuration_module (
  configuration_id,
  module_id,
  settings
)
VALUES (
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
  auto query = getQuery(R"(
DELETE FROM configuration_module
WHERE configuration_id = :configuration_id
AND module_id = :module_id;)",
    {
      {":configuration_id", configuration.value},
      {":module_id", module.value}
    });

  return exec(query);
}

bool ConfigurationRepository::updateSensorType(
  const SensorType& type)
{
  auto query = getQuery(R"(
UPDATE sensor_type
SET
  name = :name,
  description = :description,
  calibration_kind = :calibration_kind
WHERE id = :id;
  )");

  query.bindValue(
    ":id",
    type.id.value);

  query.bindValue(
    ":name",
    QString::fromStdString(type.name));

  query.bindValue(
    ":description",
    QString::fromStdString(type.description));

  query.bindValue(
    ":calibration_kind",
    static_cast<int>(type.calibrationKind));

  return exec(query);
}

bool ConfigurationRepository::createSensorType(
  const QString& name,
  SensorTypeId& id)
{
  auto query = getQuery(R"(
INSERT INTO sensor_type
  (name, description, calibration_kind)
VALUES
  (:name, '', 0);
  )");

  query.bindValue(":name", name);

  if (!exec(query))
    return false;

  const auto newId = query.lastInsertId();

  if (!newId.isValid() || newId.toUInt() == 0)
    return false;

  id = SensorTypeId{newId.toUInt()};

  return true;
}

bool ConfigurationRepository::removeSensorType(
  SensorTypeId id)
{
  auto query = getQuery(R"(
DELETE FROM sensor_type
WHERE id = :id;
  )");

  query.bindValue(":id", id.value);

  return exec(query);
}

bool ConfigurationRepository::updateSensor(
  const Sensor& sensor)
{
  auto query = getQuery(R"(
UPDATE sensor
SET
  sensor_type_id = :sensor_type_id,
  name = :name,
  serial = :serial,
  description = :description,
  calibration_mode = :calibration_mode,
  calibration_kind = :calibration_kind
WHERE id = :id;
  )");

  query.bindValue(
    ":id",
    sensor.id.value);

  query.bindValue(
    ":sensor_type_id",
    sensor.sensorTypeId.value);

  query.bindValue(
    ":name",
    QString::fromStdString(sensor.name));

  query.bindValue(
    ":serial",
    QString::fromStdString(sensor.serial));

  query.bindValue(
    ":description",
    QString::fromStdString(sensor.description));

  query.bindValue(
    ":calibration_mode",
    static_cast<int>(sensor.calibrationMode));

  query.bindValue(
    ":calibration_kind",
    static_cast<int>(sensor.calibrationKind));

  return exec(query);
}

bool ConfigurationRepository::createSensor(
  const QString& name,
  SensorTypeId typeId,
  SensorId& id)
{
  auto query = getQuery(R"(
INSERT INTO sensor
  (sensor_type_id, name)
VALUES
  (:sensor_type_id, :name);
  )");

  query.bindValue(
    ":sensor_type_id",
    typeId.value);

  query.bindValue(
    ":name",
    name);

  if (!exec(query))
    return false;

  const auto newId = query.lastInsertId();

  if (!newId.isValid() || newId.toUInt() == 0)
    return false;

  id = SensorId{newId.toUInt()};

  return true;
}

bool ConfigurationRepository::removeSensor(
  SensorId id)
{
  auto query = getQuery(R"(
DELETE FROM sensor
WHERE id = :id;
  )");

  query.bindValue(
    ":id",
    id.value);

  return exec(query);
}

bool ConfigurationRepository::createCalibration(
  Calibration& calibration)
{
  if (calibration.id)
    return false;

  // Ровно один владелец.
  if (bool(calibration.sensorId) ==
      bool(calibration.sensorTypeId))
    return false;

  Calibration validated = calibration;

  switch (validated.kind)
  {
  case CalibrationKind::Table:
    if (!validated.buildSegments())
      return false;
    break;

  case CalibrationKind::Polynomial:
    if (validated.coefficients.empty())
      return false;

    for (double value : validated.coefficients)
    {
      if (!std::isfinite(value))
        return false;
    }
    break;

  default:
    return false;
  }

  if (!m_database.transaction())
  {
    m_error = m_database.lastError();
    return false;
  }

  auto rollback = [this]()
  {
    m_database.rollback();
    return false;
  };

  auto query = getQuery(R"(
INSERT INTO calibration (
  sensor_id,
  sensor_type_id,
  kind,
  description
)
VALUES (
  :sensor_id,
  :sensor_type_id,
  :kind,
  :description
);)");

  query.bindValue(
    ":sensor_id",
    calibration.sensorId
      ? QVariant(calibration.sensorId.value)
      : QVariant());

  query.bindValue(
    ":sensor_type_id",
    calibration.sensorTypeId
      ? QVariant(calibration.sensorTypeId.value)
      : QVariant());

  query.bindValue(
    ":kind",
    static_cast<int>(calibration.kind));

  query.bindValue(
    ":description",
    calibration.description);

  if (!exec(query))
    return rollback();

  const auto id =
    query.lastInsertId().toUInt();

  if (id == 0)
    return rollback();

  if (calibration.kind == CalibrationKind::Table)
  {
    auto pointQuery = getQuery(R"(
INSERT INTO calibration_point (
  calibration_id,
  `index`,
  x,
  y
)
VALUES (
  :calibration_id,
  :index,
  :x,
  :y
);)");

    for (std::size_t i = 0;
         i < validated.points.size();
         ++i)
    {
      const auto& point = validated.points[i];

      pointQuery.bindValue(
        ":calibration_id", id);

      pointQuery.bindValue(
        ":index", static_cast<uint>(i));

      pointQuery.bindValue(
        ":x", point.x);

      pointQuery.bindValue(
        ":y", point.y);

      if (!exec(pointQuery))
        return rollback();
    }
  }
  else
  {
    auto coefficientQuery = getQuery(R"(
INSERT INTO calibration_coefficient (
  calibration_id,
  `index`,
  value
)
VALUES (
  :calibration_id,
  :index,
  :value
);)");

    for (std::size_t i = 0;
         i < validated.coefficients.size();
         ++i)
    {
      coefficientQuery.bindValue(
        ":calibration_id", id);

      coefficientQuery.bindValue(
        ":index", static_cast<uint>(i));

      coefficientQuery.bindValue(
        ":value", validated.coefficients[i]);

      if (!exec(coefficientQuery))
        return rollback();
    }
  }

  if (!m_database.commit())
  {
    m_error = m_database.lastError();
    m_database.rollback();
    return false;
  }

  calibration.id = CalibrationId{id};

  if (calibration.kind == CalibrationKind::Table)
  {
    calibration.segments =
      std::move(validated.segments);

    for (std::size_t i = 0;
         i < calibration.points.size();
         ++i)
    {
      calibration.points[i].index =
        static_cast<uint32_t>(i);
    }
  }

  return true;
}

bool ConfigurationRepository::removeCalibration(
  CalibrationId id)
{
  if (!id)
    return false;

  auto query = getQuery(R"(
DELETE FROM calibration
WHERE id = :id;)");

  query.bindValue(":id", id.value);

  if (!exec(query))
    return false;

  return query.numRowsAffected() == 1;
}

bool ConfigurationRepository::updateCalibration(
  const Calibration& calibration)
{
  if (!calibration.id)
    return false;

  // Ровно один владелец.
  if (bool(calibration.sensorId) ==
      bool(calibration.sensorTypeId))
    return false;

  // Проверяем данные тарировки.
  Calibration validated = calibration;

  switch (validated.kind)
  {
  case CalibrationKind::Table:
    if (!validated.buildSegments())
      return false;
    break;

  case CalibrationKind::Polynomial:
    if (validated.coefficients.empty())
      return false;

    for (double value : validated.coefficients)
    {
      if (!std::isfinite(value))
        return false;
    }
    break;

  default:
    return false;
  }

  if (!m_database.transaction())
  {
    m_error = m_database.lastError();
    return false;
  }

  auto rollback = [this]()
  {
    m_database.rollback();
    return false;
  };

  // Проверяем существующую тарировку и её владельца.
  auto query = getQuery(R"(
SELECT
  sensor_id,
  sensor_type_id,
  kind
FROM calibration
WHERE id = :id
FOR UPDATE;)",
    {{":id", calibration.id.value}});

  if (!exec(query))
    return rollback();

  if (!query.next())
    return rollback();

  const SensorId sensorId{
    query.value("sensor_id").toUInt()
  };

  const SensorTypeId sensorTypeId{
    query.value("sensor_type_id").toUInt()
  };

  const auto kind =
    static_cast<CalibrationKind>(
      query.value("kind").toInt());

  if (sensorId != calibration.sensorId ||
      sensorTypeId != calibration.sensorTypeId ||
      kind != calibration.kind)
  {
    return rollback();
  }

  // Обновляем только название и описание.
  query = getQuery(R"(
UPDATE calibration
SET
  description = :description
WHERE id = :id;)",
   {
     {":id", calibration.id.value},
     {":description", calibration.description}
   });

  if (!exec(query))
    return rollback();

  // Удаляем прежние данные выбранного вида.
  const QString table =
    calibration.kind == CalibrationKind::Table
      ? "calibration_point"
      : "calibration_coefficient";

  query = getQuery(
    QString("DELETE FROM %1 WHERE calibration_id = :id;")
      .arg(table),
    {{":id", calibration.id.value}});

  if (!exec(query))
    return rollback();

  // Записываем новые данные.
  if (calibration.kind == CalibrationKind::Table)
  {
    auto pointQuery = getQuery(R"(
INSERT INTO calibration_point (
  calibration_id,
  `index`,
  x,
  y
)
VALUES (
  :calibration_id,
  :index,
  :x,
  :y
);)");

    for (std::size_t i = 0;
         i < validated.points.size();
         ++i)
    {
      const auto& point = validated.points[i];

      pointQuery.bindValue(
        ":calibration_id", calibration.id.value);
      pointQuery.bindValue(
        ":index", static_cast<uint>(i));
      pointQuery.bindValue(":x", point.x);
      pointQuery.bindValue(":y", point.y);

      if (!exec(pointQuery))
        return rollback();
    }
  }
  else
  {
    auto coefficientQuery = getQuery(R"(
INSERT INTO calibration_coefficient (
  calibration_id,
  `index`,
  value
)
VALUES (
  :calibration_id,
  :index,
  :value
);)");

    for (std::size_t i = 0;
         i < validated.coefficients.size();
         ++i)
    {
      coefficientQuery.bindValue(
        ":calibration_id", calibration.id.value);
      coefficientQuery.bindValue(
        ":index", static_cast<uint>(i));
      coefficientQuery.bindValue(
        ":value", validated.coefficients[i]);

      if (!exec(coefficientQuery))
        return rollback();
    }
  }

  if (!m_database.commit())
  {
    m_error = m_database.lastError();
    m_database.rollback();
    return false;
  }

  return true;
}

bool ConfigurationRepository::sensorCalibration(
  SensorId sensorId, CalibrationKind kind,
  Calibration& calibration)
{
  calibration = {};

  auto query = getQuery(R"(
SELECT
  id,
  sensor_id,
  sensor_type_id,
  kind,
  description
FROM calibration
WHERE sensor_id = :sensor_id
  AND kind = :kind;)",
    {
      {":sensor_id", sensorId.value},
      {":kind", static_cast<int>(kind)}
    });
  if (!exec(query))
    return false;

  if (!query.next())
    return true;

  calibration.id = CalibrationId{
    query.value("id").toUInt()
  };

  calibration.sensorId = sensorId;

  calibration.kind = static_cast<CalibrationKind>(
    query.value("kind").toInt());

  calibration.description =
    query.value("description").toString();

  // Загрузка точек и коэффициентов — общий метод.
  return loadCalibrationData(calibration);
}

bool ConfigurationRepository::sensorTypeCalibration(
  SensorTypeId sensorTypeId,
  CalibrationKind kind,
  Calibration& calibration)
{
  calibration = {};

  auto query = getQuery(R"(
SELECT
  id,
  sensor_id,
  sensor_type_id,
  kind,
  description
FROM calibration
WHERE sensor_type_id = :sensor_type_id
  AND kind = :kind;)",
    {
      {":sensor_type_id", sensorTypeId.value},
      {":kind", static_cast<int>(kind)}
    });
  if (!exec(query))
    return false;

  if (!query.next())
    return true;

  calibration.id = CalibrationId{
    query.value("id").toUInt()
  };

  calibration.sensorTypeId = sensorTypeId;

  calibration.kind = static_cast<CalibrationKind>(
    query.value("kind").toInt());

  calibration.description =
    query.value("description").toString();

  return loadCalibrationData(calibration);
}


bool ConfigurationRepository::loadCalibrationData(
  Calibration& calibration)
{
  if (calibration.kind == CalibrationKind::Table)
  {
    auto query = getQuery(R"(
SELECT
  `index`,
  x,
  y
FROM calibration_point
WHERE calibration_id = :id
ORDER BY `index`;)",
                          {{":id", calibration.id.value}});

    if (!exec(query))
      return false;

    while (query.next())
    {
      calibration.points.push_back({
        query.value("index").toUInt(),
        query.value("x").toDouble(),
        query.value("y").toDouble()
      });
    }

    return calibration.buildSegments();
  }

  else if (calibration.kind == CalibrationKind::Polynomial)
  {
    auto query = getQuery(R"(
SELECT
  `index`,
  value
FROM calibration_coefficient
WHERE calibration_id = :id
ORDER BY `index`;)",
                          {{":id", calibration.id.value}});

    if (!exec(query))
      return false;

    while (query.next())
    {
      calibration.coefficients.push_back(
        query.value("value").toDouble());
    }

    return !calibration.coefficients.empty() &&
           std::ranges::all_of(
             calibration.coefficients,
             [](double value)
             {
               return std::isfinite(value);
             });
  }

  return false;
}

}
