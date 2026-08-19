#include "tst_dataserver.h"
#include "datasourcefactory.h"
#include "fakeschedulerclock.h"
#include "runtimesystem.h"
#include "systembuilder.h"
#include "systemconfiguration.h"
#include "testarchivewriter.h"
#include "testdatasource.h"
#include "testpublisher.h"
#include "testsrv.h"

tst_dataserver::tst_dataserver() { }
tst_dataserver::~tst_dataserver() = default;

void tst_dataserver::test_systemBuilder_success()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);
  cfg.addSignalDefinition({.id = {24}, .name = "D", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {2}, .formula = "A + A", .dependencies = {{17}, {17}}});

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
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
    archive,
    publisher,
    clock,
    runtime));

  const auto* c =
    cfg.findSignalDefinition(SignalId{23});
  const auto* d =
    cfg.findSignalDefinition(SignalId{24});

  QCOMPARE(
    c->formulaId,
    FormulaId{23});

  QCOMPARE(
    c->dependencies.size(),
    std::size_t(2));

  QCOMPARE(
    c->dependencies[0],
    SignalId{17});

  QCOMPARE(
    c->dependencies[1],
    SignalId{4});

  QCOMPARE(
    d->formulaId,
    FormulaId{24});

  QCOMPARE(
    d->dependencies.size(),
    std::size_t(1));

  QCOMPARE(
    d->dependencies[0],
    SignalId{17});

  QCOMPARE(
    runtime.layout.rawSignalCount(),
    2u);

  QCOMPARE(
    runtime.layout.calculatedSignalCount(),
    4u);

  QCOMPARE(
    runtime.formulas.size(),
    std::size_t(4));

  QCOMPARE(
    runtime.calculationPlan.size(),
    std::size_t(4));

  QCOMPARE(
    runtime.dataSources.size(),
    std::size_t(1));

  QVERIFY(
    runtime.signalProcessor != nullptr);

  QVERIFY(
    runtime.engine != nullptr);
}

void tst_dataserver::test_systemBuilder_process()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
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
    archive,
    publisher,
    clock,
    runtime));

  QVERIFY(
    runtime.engine->process());

  QCOMPARE(
    archive.count,
    1);

  QCOMPARE(
    publisher.count,
    1);
}

void tst_dataserver::test_systemBuilder_failErrorFormula()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  cfg.addSignalDefinition({
    .id = {30},
    .name = "D",
    .kind = SignalKind::Calculated,
    .formulaId = {2},
    .formula = "unknown + C",
    .dependencies = {{4}, {23}}
  });

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
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

  QVERIFY(!builder.build(
    cfg,
    factory,
    archive,
    publisher,
    clock,
    runtime));
}

void tst_dataserver::test_systemBuilder_failDataSourceManager()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::LCard);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
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

  QVERIFY(!builder.build(
    cfg,
    factory,
    archive,
    publisher,
    clock,
    runtime));
}

void tst_dataserver::test_systemBuilder_cycle()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  TestPublisher publisher;
  FakeSchedulerClock clock(2, 5);

  RuntimeSystem runtime;

  SystemBuilder builder;

  QVERIFY(builder.build(
    cfg,
    factory,
    archive,
    publisher,
    clock,
    runtime));

  for (int n = 0; n < 1000; ++n)
  {
    QVERIFY(runtime.engine->process());

    auto count = n + 1;
    double a = n;
    double b = n * 10;
    QCOMPARE(archive.count, count);
    QCOMPARE(publisher.count, count);

    const auto& archived = archive.last();
    const auto& published = publisher.last();

    QCOMPARE(archived.number, FrameNumber{static_cast<uint64_t>(count)});

    QCOMPARE(archived.timestamp, Timestamp{static_cast<uint64_t>(count * 2)});

    QCOMPARE(archived.wallTime, WallClockTime{static_cast<int64_t>(count * 5)});

    QCOMPARE(archived.raw().valueRef(0), a);
    QCOMPARE(archived.raw().valueRef(1), b);

    QCOMPARE(archived.calculated().valueRef(0), a);
    QCOMPARE(archived.calculated().valueRef(1), b);
    QCOMPARE(archived.calculated().valueRef(2), a + b);

    QCOMPARE(
      published.raw().valueRef(0),
      archived.raw().valueRef(0));

    QCOMPARE(
      published.raw().valueRef(1),
      archived.raw().valueRef(1));

    QCOMPARE(
      published.calculated().valueRef(0),
      archived.calculated().valueRef(0));

    QCOMPARE(
      published.calculated().valueRef(1),
      archived.calculated().valueRef(1));

    QCOMPARE(
      published.calculated().valueRef(2),
      archived.calculated().valueRef(2));
  }
}