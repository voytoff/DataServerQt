#include "tst_configeditor.h"
#include "qds/db.h"
#include <qtestcase.h>
#include <QJsonDocument>
#include <QByteArray>
#include <QSqlQuery>
#include "db/configurationrepository.h"
#include <vector>
#include <optional>
#include <algorithm>

tst_configeditor::tst_configeditor() { }
tst_configeditor::~tst_configeditor() = default;

void tst_configeditor::test_configurationRepository_moduleLifecycle()
{
  using namespace qds;

  auto db = get_db();

  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  QSqlQuery query("DELETE FROM module WHERE serial='TESTMODULE51';", db);
  QVERIFY(query.exec());

  ConfigurationRepository repo(db);

  std::vector<AvailableModule> before;

  QVERIFY(
    repo.availableModules(
      ConfigurationId{1},
      before));

  const auto module =
    repo.addModule(
      CrateId{1},
      ModuleType::LTR51,
      "TESTMODULE51",
      31,
      "Модуль для тестирования каскадного удаления");

  QVERIFY(module.has_value());

  std::vector<AvailableModule> afterCreate;

  QVERIFY(repo.availableModules(
    ConfigurationId{1},
    afterCreate));

  QCOMPARE(
    afterCreate.size(),
    before.size() + 1);

  QVERIFY(
    repo.addConfigModule(
      ConfigurationId{1},
      *module));

  std::vector<AvailableModule> afterAdd;

  QVERIFY(repo.availableModules(
    ConfigurationId{1},
    afterAdd));

  QCOMPARE(
    afterAdd.size(),
    before.size());

  auto ok =
    repo.removeModule(
      *module);

  if (!ok) {
    qDebug() << repo.lastError().text();
    QFAIL("removeModule failed");
  }

  std::vector<AvailableModule> afterRemove;

  QVERIFY(repo.availableModules(
    ConfigurationId{1},
    afterRemove));

  QCOMPARE(
    afterRemove.size(),
    before.size());
}

void tst_configeditor::test_configurationRepository_moduleLifecycle_withTags()
{
  using namespace qds;

  auto db = get_db();

  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  QSqlQuery query("DELETE FROM module WHERE serial='TESTMODULE51';", db);
  QVERIFY(query.exec());

  ConfigurationRepository repo(db);

  const auto module =
    repo.addModule(
      CrateId{1},
      ModuleType::LTR51,
      "TESTMODULE51",
      15,
      "Модуль для тестирования каскадного удаления");

  QVERIFY(module.has_value());

  QVERIFY(
    repo.addConfigModule(
      ConfigurationId{1},
      *module));

  QJsonObject json{
    {"mode", 2},
    {"range", 1}
  };

  const auto tag =
    repo.addConfigTag(
      ConfigurationId{1},
      *module,
      ChannelId{13},
      json);

  QVERIFY(tag.has_value());

  std::vector<ConfigurationTag> tags;

  QVERIFY(
    repo.moduleConfigTags(
      ConfigurationId{1},
      *module,
      tags));

  QCOMPARE(tags.size(), 1);

  const auto tag1 = tags[0];

  QCOMPARE(tag1.tag, *tag);
  QCOMPARE(tag1.module, *module);
  QCOMPARE(tag1.channel, ChannelId{13});
  QCOMPARE(tag1.settings, json);

  // При наличии configuration_module и configuration_tag
  // физический модуль должен удалиться каскадно
  // вместе со всеми зависимыми записями.
  QVERIFY(
    repo.removeModule(
      *module));
}

void tst_configeditor::test_configurationRepository_configModules()
{
  using namespace qds;

  auto db = get_db();

  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  QSqlQuery query("DELETE FROM module WHERE serial='TESTMODULE51';", db);
  QVERIFY(query.exec());

  ConfigurationRepository repo(db);

  std::vector<ConfigModule> configModules;

  auto findTestModule = [](const std::vector<ConfigModule>& modules) -> const ConfigModule* {
    auto it = std::find_if(modules.begin(), modules.end(), [](const ConfigModule& m) {
      return m.moduleSerial == "TESTMODULE51";
    });

    return (it != modules.end()) ? &(*it) : nullptr;
  };

  QVERIFY(
    repo.configModules(
      ConfigurationId{1},
      configModules));

  auto m = findTestModule(configModules);

  QVERIFY(!m);

  const auto module =
    repo.addModule(
      CrateId{1},
      ModuleType::LTR51,
      "TESTMODULE51",
      15,
      "Модуль для тестирования каскадного удаления");

  QVERIFY(module.has_value());

  QVERIFY(
    repo.addConfigModule(
      ConfigurationId{1},
      *module));

  QVERIFY(
    repo.configModules(
      ConfigurationId{1},
      configModules));

  m = findTestModule(configModules);

  QVERIFY(m);

  QCOMPARE(
    m->module,
    module.value());

  QCOMPARE(
    m->settings,
    QJsonObject{});

  QVERIFY(
    repo.removeModule(
      *module));

  QVERIFY(
    repo.configModules(
      ConfigurationId{1},
      configModules));

  m = findTestModule(configModules);

  QVERIFY(!m);
}