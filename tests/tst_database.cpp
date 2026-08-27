#include "tst_database.h"
#include "configurationrepository.h"
#include "db.h"
#include "systemconfiguration.h"
#include "testsrv.h"
#include <QSqlTableModel>
#include <qtestcase.h>

tst_database::tst_database() { }
tst_database::~tst_database() = default;

void tst_database::test_database_loadConfiguration()
{
  using namespace qds;
  auto db = get_db();
  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repo(db);

  SystemConfiguration cfg;

  QVERIFY(repo.load(ConfigurationId{1}, cfg));

  QCOMPARE(cfg.udpPort(), 5001);
  QCOMPARE(cfg.name(), "Иследование 1");
  QCOMPARE(cfg.description(), "Тестовая конфигурация");

  QCOMPARE(cfg.crates().size(), 1);
  QCOMPARE(cfg.modules().size(), 1);
  QCOMPARE(cfg.signalDefinitions().size(), 5);

  const CrateInfo &ci = cfg.crates()[0];
  QCOMPARE(ci.type, CrateType::LTR_EU_16_1); // в базе - 1

  QCOMPARE(
    ci.serial,
    "3T778029");

  QCOMPARE(
    ci.host,
    "127.0.0.1");

  QCOMPARE(
    ci.port,
    11111u);

  QCOMPARE(
    ci.description,
    "CRATE16");

  const ModuleInfo &mi = cfg.modules()[0];

  QCOMPARE(mi.crate, ci.id);
  QCOMPARE(mi.type, ModuleType::LTR11);

  QCOMPARE(
    mi.serial,
    "LTR11-000001");

  QCOMPARE(
    mi.description,
    "LTR11 test module");

  QCOMPARE(
    mi.settings,
    QJsonObject{});

  auto mtags = cfg.moduleTags(mi.id);
  QCOMPARE(mtags.size(), 2);

  auto tags = cfg.tags();
  QCOMPARE(tags.size(), 2);

  QCOMPARE(tags[0].channel, {0});
  QCOMPARE(tags[1].channel, {1});

  auto ss = cfg.signalDefinitions();
  QCOMPARE(ss.size(), 5);

  auto raw0 = findSignalDefinition(ss, "Raw0");
  QVERIFY(raw0);
  QCOMPARE(raw0->source.tag, mtags[0]);
  QCOMPARE(raw0->archiveFrequency, 1000);
  QCOMPARE(raw0->kind, SignalKind::Raw);
  QCOMPARE(raw0->calibrationMode, CalibrationMode::None);
  QCOMPARE(raw0->formula, "");
  QCOMPARE(raw0->signalType, SignalTypeId{1});

  auto raw1 = findSignalDefinition(ss, "Raw1");
  QVERIFY(raw1);
  QCOMPARE(raw1->source.tag, mtags[1]);
  QCOMPARE(raw1->archiveFrequency, 100);
  QCOMPARE(raw1->kind, SignalKind::Raw);
  QCOMPARE(raw1->calibrationMode, CalibrationMode::None);
  QCOMPARE(raw1->formula, "");
  QCOMPARE(raw1->signalType, SignalTypeId{2});

  auto a = findSignalDefinition(ss, "A");
  QVERIFY(a);
  QCOMPARE(a->archiveFrequency, 100);
  QCOMPARE(a->kind, SignalKind::Calculated);
  QCOMPARE(a->calibrationMode, CalibrationMode::BySignal);
  QCOMPARE(a->formula, "Raw0");

  auto b = findSignalDefinition(ss, "B");
  QVERIFY(b);
  QCOMPARE(b->archiveFrequency, 10);
  QCOMPARE(b->kind, SignalKind::Calculated);
  QCOMPARE(b->calibrationMode, CalibrationMode::BySignalType);
  QCOMPARE(b->formula, "Raw1");

  auto c = findSignalDefinition(ss, "C");
  QVERIFY(c);
  QCOMPARE(c->archiveFrequency, 10);
  QCOMPARE(c->kind, SignalKind::Calculated);
  QCOMPARE(c->calibrationMode, CalibrationMode::None);
  QCOMPARE(c->formula, "A + B");
}
