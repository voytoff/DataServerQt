#include "tst_database.h"
#include "configurationrepository.h"
#include "datasourcefactory.h"
#include "db.h"
#include "fakeschedulerclock.h"
#include "runtimesystem.h"
#include "systembuilder.h"
#include "systemconfiguration.h"
#include "testarchivewriter.h"
#include "testdatasource.h"
#include "testpublisher.h"
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
  QCOMPARE(a->signalType, SignalTypeId{1});

  auto b = findSignalDefinition(ss, "B");
  QVERIFY(b);
  QCOMPARE(b->archiveFrequency, 10);
  QCOMPARE(b->kind, SignalKind::Calculated);
  QCOMPARE(b->calibrationMode, CalibrationMode::BySignalType);
  QCOMPARE(b->formula, "Raw1");
  QCOMPARE(b->signalType, SignalTypeId{2});

  auto c = findSignalDefinition(ss, "C");
  QVERIFY(c);
  QCOMPARE(c->archiveFrequency, 10);
  QCOMPARE(c->kind, SignalKind::Calculated);
  QCOMPARE(c->calibrationMode, CalibrationMode::None);
  QCOMPARE(c->formula, "A + B");
  QCOMPARE(c->signalType, SignalTypeId{2});
}

void tst_database::test_database_loadCalibrations()
{
  using namespace qds;
  auto db = get_db();
  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repo(db);

  SystemConfiguration cfg;
  QVERIFY(repo.load(ConfigurationId{1}, cfg));

  CalibrationRepository cr;
  QVERIFY(repo.loadCalibrations(cfg, cr));

  QCOMPARE(cr.sizeSignals(), 1);
  QCOMPARE(cr.sizeSignalTypes(), 1);

  auto definitions = cfg.signalDefinitions();
  double result = 0;

  auto a = findSignalDefinition(definitions, "A");

  QVERIFY(cr.calibrateBySignal(a->id, -10, result));
  QCOMPARE(result, -1.0);

  QVERIFY(cr.calibrateBySignal(a->id, 0, result));
  QCOMPARE(result, 0.0);

  QVERIFY(cr.calibrateBySignal(a->id, 5, result));
  QCOMPARE(result, 0.5);

  QVERIFY(cr.calibrateBySignal(a->id, 10, result));
  QCOMPARE(result, 1.0);

  QVERIFY(cr.calibrateBySignal(a->id, 15, result));
  QCOMPARE(result, 1.5);

  QVERIFY(cr.calibrateBySignal(a->id, 20, result));
  QCOMPARE(result, 2.0);

  QVERIFY(cr.calibrateBySignal(a->id, 30, result));
  QCOMPARE(result, 3.0);

  auto b = SignalTypeId{2}; // B - имеет тип с идентификатором 2

  QVERIFY(cr.calibrateBySignalType(b, -10, result));
  QCOMPARE(result, -30.0);

  QVERIFY(cr.calibrateBySignalType(b, 0, result));
  QCOMPARE(result, -20.0);

  QVERIFY(cr.calibrateBySignalType(b, 5, result));
  QCOMPARE(result, -15.0);

  QVERIFY(cr.calibrateBySignalType(b, 10, result));
  QCOMPARE(result, -10.0);

  QVERIFY(cr.calibrateBySignalType(b, 15, result));
  QCOMPARE(result, -5.0);

  QVERIFY(cr.calibrateBySignalType(b, 20, result));
  QCOMPARE(result, 0.0);

  QVERIFY(cr.calibrateBySignalType(b, 30, result));
  QCOMPARE(result, 10.0);

  result = 123.0;

  QVERIFY(
    !cr.calibrateBySignal(
      SignalId{999},
      10.0,
      result));
  QCOMPARE(result, 123.0);


  QVERIFY(
    !cr.calibrateBySignalType(
      SignalTypeId{999},
      10.0,
      result));
  QCOMPARE(result, 123.0);

}

void tst_database::test_database_failLoading()
{
  using namespace qds;
  auto db = get_db();
  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repo(db);

  SystemConfiguration cfg;

  QVERIFY(repo.load(ConfigurationId{1}, cfg));

  auto c = findSignalDefinition(cfg.signalDefinitions(), "C");

  QSqlQuery query(get_db());

  QVERIFY(query.prepare(R"(
UPDATE configuration_signal_definition
SET calibration_mode=1
WHERE id=:id;
)"));

  query.bindValue(":id", c->id.value);

  QVERIFY(query.exec());

  QVERIFY(repo.load(ConfigurationId{1}, cfg));

  CalibrationRepository cr;
  QVERIFY(!repo.loadCalibrations(cfg, cr));

  QCOMPARE(cr.sizeSignals(), 0);
  QCOMPARE(cr.sizeSignalTypes(), 0);

  QVERIFY(query.prepare(R"(
UPDATE configuration_signal_definition
SET calibration_mode=0
WHERE id=:id;
)"));

  query.bindValue(":id", c->id.value);

  QVERIFY(query.exec());

  QVERIFY(repo.load(ConfigurationId{1}, cfg));

  QVERIFY(repo.loadCalibrations(cfg, cr));

  QCOMPARE(cr.sizeSignals(), 1);
  QCOMPARE(cr.sizeSignalTypes(), 1);
}

void tst_database::test_database_pipeline()
{
  using namespace qds;
  auto db = get_db();
  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repo(db);

  SystemConfiguration cfg;
  QVERIFY(repo.load(ConfigurationId{1}, cfg));

  CalibrationRepository cr;
  QVERIFY(repo.loadCalibrations(cfg, cr));

  QCOMPARE(cr.sizeSignals(), 1);
  QCOMPARE(cr.sizeSignalTypes(), 1);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::LTR11,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  TestPublisher publisher;
  FakeSchedulerClock clock;

  RuntimeSystem runtime;

  SystemBuilder builder;

  QVERIFY(builder.build(
    cfg,
    factory,
    cr,
    runtime));

  QVERIFY(runtime.engine->initialize(
    runtime.dataSources,
    *runtime.signalProcessor,
    runtime.buffers,
    archive,
    publisher,
    clock));

  QCOMPARE(runtime.calibrations.sizeSignals(), 1);
  QCOMPARE(runtime.calibrations.sizeSignalTypes(), 1);

  QVERIFY(runtime.engine->process());

  const auto &frame0 = runtime.buffers.readFrame();

  QCOMPARE(frame0.raw().valueRef(0), 0.0);
  QCOMPARE(frame0.raw().valueRef(1), 0.0);

  QCOMPARE(frame0.calculated().valueRef(0), 0.0);
  QCOMPARE(frame0.calculated().valueRef(1), -20.0);
  QCOMPARE(frame0.calculated().valueRef(2), 0.0 + -20.0);

  QVERIFY(runtime.engine->process());

  const auto &frame1 = runtime.buffers.readFrame();

  QCOMPARE(frame1.raw().valueRef(0), 1.0);
  QCOMPARE(frame1.raw().valueRef(1), 10.0);

  QCOMPARE(frame1.calculated().valueRef(0), 0.1);
  QCOMPARE(frame1.calculated().valueRef(1), -10.0);
  QCOMPARE(frame1.calculated().valueRef(2), 0.1 + -10.0);

  QVERIFY(runtime.engine->process());

  const auto &frame2 = runtime.buffers.readFrame();

  QCOMPARE(frame2.raw().valueRef(0), 2.0);
  QCOMPARE(frame2.raw().valueRef(1), 20.0);

  QCOMPARE(frame2.calculated().valueRef(0), 0.2);
  QCOMPARE(frame2.calculated().valueRef(1), 0.0);
  QCOMPARE(frame2.calculated().valueRef(2), 0.2 + 0.0);
}
