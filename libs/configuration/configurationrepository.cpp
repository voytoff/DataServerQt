#include "configurationrepository.h"
#include <QSqlQuery>
#include <QSqlError>

namespace qds
{

ConfigurationRepository::ConfigurationRepository(
  const QSqlDatabase &db)
  : m_db(db) { }

bool ConfigurationRepository::load(ConfigurationId id, SystemConfiguration &configuration)
{
  SystemConfiguration cfg;

  // проверяем наличие конфигурации
  auto query = getQuery(
    "SELECT id, name, description, udp_port FROM configuration WHERE id=:id;",
    {{":id", id.value}});
  if (!query.exec() || !query.next())
    return false;

  cfg.setUdpPort(query.value("udp_port").toUInt());

  cfg.setName(query.value("name").toString().toStdString());

  cfg.setDescription(query.value("description").toString().toStdString());

  // 1 загружаем модули
  query = getQuery(R"(
SELECT
  cm.configuration_id,
  cm.module_id,
  cm.settings,
  m.type as module_type,
  m.serial as module_serial,
  m.description as module_description,
  c.id as crate_id,
  c.type as crate_type,
  c.serial as crate_serial,
  c.host as crate_host,
  c.port as crate_port,
  c.description as crate_description
FROM
  configuration_module cm
JOIN module m on
  m.id = cm.module_id
JOIN crate c on
  c.id = m.crate_id
WHERE
  cm.configuration_id = :id;)",
    {{":id", id.value}});
  if (!query.exec()) return false;

  while (query.next())
  {
    ModuleInfo module;
    module.id = ModuleId{query.value("module_id").toUInt()};

    const QByteArray data =
      query.value("settings")
        .toString()
        .toUtf8();

    const QJsonDocument document =
      QJsonDocument::fromJson(data);

    if (!document.isObject())
      return false;

    module.settings =
      document.object();

    // информация о модуле из module
    module.type = static_cast<ModuleType>(query.value("module_type").toUInt());
    module.crate = CrateId{query.value("crate_id").toUInt()};
    module.serial = query.value("module_serial").toString();
    module.description = query.value("module_description").toString();

    auto crates = cfg.crates();
    auto it = std::find_if(crates.begin(), crates.end(), [&](const CrateInfo &ci) {return ci.id == module.crate;});

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

    if (!cfg.addModule(module)) return false;
  }

  if (cfg.modules().size() == 0) return false;

  // загружаем теги
  query = getQuery(
    "SELECT id, configuration_id, module_id, channel FROM configuration_tag WHERE configuration_id=:id;",
    {{":id", id.value}});
  if (!query.exec()) return false;

  while (query.next())
  {
    TagInfo tag;

    tag.tag = TagId{query.value("id").toUInt()};

    auto module_id = query.value("module_id").toUInt();
    auto modules = cfg.modules();
    auto it = std::find_if(modules.begin(), modules.end(), [&](const ModuleInfo &mi) {return mi.id.value == module_id;});

    if (it == modules.end()) return false;

    tag.module = it->id;
    tag.channel = ChannelId{query.value("channel").toUInt()};

    if (!cfg.addTag(tag))
      return false;
  }

  if (cfg.tags().size() == 0) return false;

  // загружаем сигналы
  query = getQuery(
    "SELECT id, configuration_id, name, kind, tag_id, signal_type_id, archive_frequency, calibration_mode, formula FROM configuration_signal_definition WHERE configuration_id=:id;",
    {{":id", id.value}});
  if (!query.exec()) return false;

  while (query.next())
  {
    SignalDefinition definition;

    definition.id = SignalId{query.value("id").toUInt()};
    definition.name = query.value("name").toString().toStdString();
    definition.archiveFrequency = query.value("archive_frequency").toUInt();
    definition.kind = static_cast<SignalKind>(query.value("kind").toUInt());
    definition.signalType = SignalTypeId{query.value("signal_type_id").toUInt()};

    if (definition.kind == SignalKind::Raw) {
      definition.source = SignalSource{TagId{query.value("tag_id").toUInt()}};
    } else if (definition.kind == SignalKind::Calculated) {
      definition.formula = query.value("formula").toString().toStdString();
      definition.calibrationMode = static_cast<CalibrationMode>(query.value("calibration_mode").toUInt());
    } else return false;

    if (!cfg.addSignalDefinition(definition))
      return false;
  }

  configuration = cfg;

  return true;
}

QSqlQuery ConfigurationRepository::getQuery(
  const QString& sql,
  const QVariantMap& args)
{
  QSqlQuery query(m_db);

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

}
