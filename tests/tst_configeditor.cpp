#include "tst_configeditor.h"
#include "qds/db.h"
#include <qtestcase.h>
#include "db/configurationrepository.h"

tst_configeditor::tst_configeditor() { }
tst_configeditor::~tst_configeditor() = default;

void tst_configeditor::test_configurationRepository_configModule()
{
  using namespace qds;

  auto db = get_db();

  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

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