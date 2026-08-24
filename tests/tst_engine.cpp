#include "tst_engine.h"
#include "buffermanager.h"
#include "calculationcompiler.h"
#include "dataengine.h"
#include "failingarchivewriter.h"
#include "failingdatasource.h"
#include "fakeclock.h"
#include "fakedatasource.h"
#include "fakeschedulerclock.h"
#include "formulaadd.h"
#include "formulacopy.h"
#include "formularepository.h"
#include "nullframepublisher.h"
#include "schedulerclock.h"
#include "testarchivewriter.h"
#include "testdatasource.h"
#include "testsrv.h"
#include <qtestcase.h>

tst_engine::tst_engine() { }
tst_engine::~tst_engine() = default;
/*
void tst_engine::test_pipeline_archive_copy()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_Copy_Add(ModuleType::Fake);
  SignalMemoryLayout layout;
  layout.build(cfg);

  CalculationPlan plan;

  CalculationProcessor processor(plan);

  BufferManager buffers;
  buffers.initialize(layout);

  DataSourceFactory factory;
  QVERIFY(factory.registerType(
    ModuleType::Fake,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<FakeDataSource>(
        cfg.settings);
    }));

  DataSourceManager manager;
  QVERIFY(manager.initialize(cfg, layout, factory));

  TestArchiveWriter archive;
  NullFramePublisher publisher;

  DataEngine engine;

  FakeSchedulerClock clock;

  QVERIFY(engine.initialize(
    manager,
    processor,
    buffers,
    archive,
    publisher,
    clock));

  QVERIFY(engine.process());

  const auto archived = archive.last();

  QVERIFY(engine.process());

  QCOMPARE(archived.number.value, 1);
  QCOMPARE(archived.timestamp.value, 10);
  QCOMPARE(archived.wallTime.unixMicroseconds, 100);

  QCOMPARE(archived.raw().value(0), 1.);
  QCOMPARE(archived.raw().value(1), 2.);

  QCOMPARE(archived.calculated().value(0), 0.0);
  QCOMPARE(archived.calculated().value(1), 0.0);
  QCOMPARE(archived.calculated().value(2), 0.0);
}

void tst_engine::test_dataEngine_simple_runtime()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_Copy_Add(ModuleType::Test);
  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaRepository repo;
  QVERIFY(repo.add({0}, std::make_unique<FormulaCopy>()));
  QVERIFY(repo.add({2}, std::make_unique<FormulaAdd>()));

  CalculationCompiler compiler(cfg, layout, repo);

  CalculationPlan plan; // 0 - 0 канал; 1 - 1 канал; 2 - их сумма
  QVERIFY(compiler.build(plan));

  CalculationProcessor processor(plan);

  BufferManager buffers;
  buffers.initialize(layout);

  DataSourceFactory factory;
  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>( // две ячейки -> 0 - счетчик; 1 - счетчик * 10
        cfg.settings);
    }));

  DataSourceManager manager;
  QVERIFY(manager.initialize(cfg, layout, factory));

  TestArchiveWriter archive;
  NullFramePublisher publisher;

  DataEngine engine;

  uint64_t timestampStep = 5;
  uint64_t wallClockStep = 75;

  FakeSchedulerClock clock(timestampStep, wallClockStep);

  QVERIFY(engine.initialize(
    manager,
    processor,
    buffers,
    archive,
    publisher,
    clock));

  for (int i = 0; i < 10; i++)
  {
    QCOMPARE(archive.count, i);
    QVERIFY(engine.process());
    QCOMPARE(archive.count, i+1);

    const auto &frame = buffers.readFrame();

    const auto expectedFrame = static_cast<uint64_t>(i + 1);

    QCOMPARE(frame.number.value, expectedFrame);
    QCOMPARE(frame.timestamp.value, expectedFrame * timestampStep);
    QCOMPARE(frame.wallTime.unixMicroseconds, expectedFrame * wallClockStep);

    QCOMPARE(frame.raw().value(0), i);
    QCOMPARE(frame.raw().value(1), i * 10);

    QCOMPARE(frame.calculated().value(0), frame.raw().value(0));
    QCOMPARE(frame.calculated().value(1), frame.raw().value(1));
    QCOMPARE(frame.calculated().value(2), frame.raw().value(0) + frame.raw().value(1));

    auto p = archive.last();
    QCOMPARE(frame.calculated().value(0), p.calculated().value(0));
    QCOMPARE(frame.calculated().value(1), p.calculated().value(1));
    QCOMPARE(frame.calculated().value(2), p.calculated().value(2));

    QCOMPARE(p.number.value, frame.number.value);
    QCOMPARE(p.timestamp.value, frame.timestamp.value);
    QCOMPARE(p.wallTime.unixMicroseconds, frame.wallTime.unixMicroseconds);
  }
}

void tst_engine::test_dataEngine_FailingDataSource()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_Copy_Add(ModuleType::Fail);
  SignalMemoryLayout layout;
  layout.build(cfg);

  CalculationPlan plan;

  CalculationProcessor processor(plan);

  BufferManager buffers;
  buffers.initialize(layout);

  DataSourceFactory factory;
  QVERIFY(factory.registerType(
    ModuleType::Fail,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<FailingDataSource>(
        cfg.settings);
    }));

  DataSourceManager manager;
  QVERIFY(manager.initialize(cfg, layout, factory));

  TestArchiveWriter archive;
  NullFramePublisher publisher;

  DataEngine engine;

  FakeClock fclock;
  SchedulerClock clock(fclock);

  QVERIFY(engine.initialize(
    manager,
    processor,
    buffers,
    archive,
    publisher,
    clock));

  QVERIFY(!engine.process());
  QCOMPARE(archive.count, 0);
}

void tst_engine::test_dataEngine_FailingCalculationProcessor()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_Copy_Add(ModuleType::Test);
  SignalMemoryLayout layout;
  layout.build(cfg);

  FailingProcessor processor;

  BufferManager buffers;
  buffers.initialize(layout);

  DataSourceFactory factory;
  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  DataSourceManager manager;
  QVERIFY(manager.initialize(cfg, layout, factory));

  TestArchiveWriter archive;
  NullFramePublisher publisher;

  DataEngine engine;

  FakeClock fclock;
  SchedulerClock clock(fclock);

  QVERIFY(engine.initialize(
    manager,
    processor,
    buffers,
    archive,
    publisher,
    clock));

  QVERIFY(!engine.process());
  QCOMPARE(archive.count, 0);
}

void tst_engine::test_dataEngine_FailingArchiveWriter()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_Copy_Add(ModuleType::Test);
  SignalMemoryLayout layout;
  layout.build(cfg);

  CalculationPlan plan;

  CalculationProcessor processor(plan);

  BufferManager buffers;
  buffers.initialize(layout);

  DataSourceFactory factory;
  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  DataSourceManager manager;
  QVERIFY(manager.initialize(cfg, layout, factory));

  FailingArchiveWriter archive;
  NullFramePublisher publisher;

  DataEngine engine;

  FakeClock fclock;
  SchedulerClock clock(fclock);

  QVERIFY(engine.initialize(
    manager,
    processor,
    buffers,
    archive,
    publisher,
    clock));

  QVERIFY(!engine.process());
}

void tst_engine::test_dataEngine_simple_pipeline()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  CalculationPlan plan;
  CalculationProcessor processor(plan);

  BufferManager buffers;
  buffers.initialize(layout);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Fake,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<FakeDataSource>(
        cfg.settings);
    }));

  DataSourceManager manager;

  QVERIFY(manager.initialize(
    cfg,
    layout,
    factory));

  TestArchiveWriter archive;
  NullFramePublisher publisher;

  FakeSchedulerClock clock;

  DataEngine engine;

  QVERIFY(engine.initialize(
    manager,
    processor,
    buffers,
    archive,
    publisher,
    clock));

  QVERIFY(engine.process());

  const auto archived = archive.last();

  QVERIFY(engine.process());

  QCOMPARE(archived.number.value, 1u);
  QCOMPARE(archived.timestamp.value, 10u);
  QCOMPARE(archived.wallTime.unixMicroseconds, 100u);

  QCOMPARE(archived.raw().value(0), 1.0);
  QCOMPARE(archived.raw().value(1), 2.0);
  QCOMPARE(archived.raw().value(2), 1.0);
  QCOMPARE(archived.raw().value(3), 2.0);
  QCOMPARE(archived.raw().value(4), 3.0);
  QCOMPARE(archived.raw().value(5), 1.0);
  QCOMPARE(archived.raw().value(6), 2.0);
}
void tst_engine::test_dataEngine_withoutSources()
{
  using namespace qds;
  SystemConfiguration cfg;
  TestSrv srv(cfg);
  DataSourceManager manager;
  DataEngine engine(manager, srv.scheduler);

  QVERIFY(engine.start());
  QVERIFY(!engine.start());

  QVERIFY(engine.isRunning());
  QVERIFY(engine.step());

  engine.stop();

  QVERIFY(!engine.isRunning());
  QVERIFY(!engine.step());

  QVERIFY(engine.start());
  QVERIFY(engine.isRunning());

  engine.stop();

  QVERIFY(!engine.isRunning());
}

void tst_engine::test_dataEngine_withFakeSource()
{
  using namespace qds;
  SystemConfiguration cfg;
  TestSrv srv(cfg);

  auto ptr = std::make_unique<FakeActiveDataSource>();
  FakeActiveDataSource* src = ptr.get();

  DataSourceManager manager;
  QVERIFY(manager.add(std::move(ptr)));

  DataEngine engine(manager, srv.scheduler);

  QVERIFY(!engine.isRunning());

  QVERIFY(engine.start());

  QVERIFY(engine.isRunning());

  QVERIFY(engine.step());

  engine.stop();

  QVERIFY(!engine.isRunning());

  QCOMPARE(src->startCalls, 1);
  QCOMPARE(src->stepCalls, 1);
  QCOMPARE(src->stopCalls, 1);
}

void tst_engine::test_dataEngine_stepWithoutStart()
{
  using namespace qds;
  SystemConfiguration cfg;
  TestSrv srv(cfg);
  DataSourceManager manager;
  DataEngine engine(manager, srv.scheduler);

  QVERIFY(!engine.step());
}

void tst_engine::test_dataEngine_callSomeStops()
{
  using namespace qds;
  SystemConfiguration cfg;
  TestSrv srv(cfg);
  DataSourceManager manager;
  DataEngine engine(manager, srv.scheduler);

  QVERIFY(!engine.step());

  QVERIFY(engine.start());

  QVERIFY(engine.isRunning());

  engine.stop();

  QVERIFY(!engine.isRunning());

  engine.stop();
  QVERIFY(!engine.isRunning());

  engine.stop();
  QVERIFY(!engine.isRunning());
}

void tst_engine::test_dataEngine_restart()
{
  using namespace qds;
  SystemConfiguration cfg;
  TestSrv srv(cfg);
  DataSourceManager manager;
  DataEngine engine(manager, srv.scheduler);

  QVERIFY(engine.start());
  QVERIFY(engine.isRunning());

  engine.stop();
  QVERIFY(!engine.isRunning());

  QVERIFY(engine.start());
  QVERIFY(engine.isRunning());

  QVERIFY(engine.step());

  engine.stop();
  QVERIFY(!engine.isRunning());
}

void tst_engine::test_dataEngine_storagePipeline()
{
  using namespace qds;
  // создаем конфигурацию с одним модулем и несколькими тегами
  constexpr TagId tags[] { {0}, {1}, {2} };
  SystemConfiguration cfg = createTestConfig(tags, std::size(tags));
  // тестовый контейнер
  TestSrv srv(cfg);
  // часы
  FakeClock clock;
  // источник данных
  auto ptr = std::make_unique<GeneratorDataSource>(srv.storage, cfg, clock);
  auto source = ptr.get();
  // диспетчер данных
  DataSourceManager manager;
  QVERIFY(manager.add(std::move(ptr)));

  Subscription sub;
  sub.rate = PublishRate::Hz1;
  sub.tags = {{0},{1},{2}};

  auto id = srv.manager.add(sub);

  srv.scheduler.addSubscription(
    id,
    PublishRate::Hz1);

  DataEngine engine(manager, srv.scheduler);

  QVERIFY(engine.start());
  QVERIFY(engine.isRunning());

  uint64_t t = 1234567;

  clock.setTimestamp(0);
  clock.advance(t);

  QVERIFY(engine.step());

  QCOMPARE(source->generationCount(), 1u);


  QCOMPARE(srv.storage.sample(tags[0]).value, 0.f);

  QCOMPARE(srv.storage.sample(tags[1]).value, 1.f);

  QCOMPARE(srv.storage.sample(tags[2]).value, 2.f);

  QCOMPARE(srv.storage.timestamp(tags[0]), t);

  engine.stop();
  QVERIFY(!engine.isRunning());

  PacketWriter writer;
  srv.publisher.publish(srv.storage, sub, 777u, writer);

  PacketReader reader;
  reader.append(writer.data(), writer.size());

  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::LiveData);

  PublishHeader ldh;
  QVERIFY(reader.read(ldh));

  QCOMPARE(ldh.subscriptionId.value, 0u);
  QCOMPARE(ldh.sequence, 777u);
  QCOMPARE(ldh.timestamp, t);
  QCOMPARE(ldh.valueCount, sub.tags.size());

  std::array<Sample, 3> samples;

  QVERIFY(reader.readArray(samples.data(), samples.size()));

  QCOMPARE(samples[0].value, 0.f);
  QCOMPARE(samples[1].value, 1.f);
  QCOMPARE(samples[2].value, 2.f);

  QCOMPARE(reader.remaining(), std::size_t(0));
}

void tst_engine::test_dataEngine_completePipeline()
{
  using namespace qds;
  // создаем конфигурацию с одним модулем и несколькими тегами
  constexpr TagId tags[] { {0}, {1}, {2} };
  SystemConfiguration cfg = createTestConfig(tags, std::size(tags));
  // тестовый контейнер
  TestSrv srv(cfg);
  // часы
  FakeClock clock;
  // источник данных
  auto ptr = std::make_unique<GeneratorDataSource>(srv.storage, cfg, clock);
  auto source = ptr.get();
  // диспетчер данных
  DataSourceManager manager;
  QVERIFY(manager.add(std::move(ptr)));

  Subscription sub;
  sub.rate = PublishRate::Hz1;
  sub.tags = {{0},{1},{2}};

  auto id = srv.manager.add(sub);

  srv.scheduler.addSubscription(
    id,
    PublishRate::Hz1);

  DataEngine engine(manager, srv.scheduler);

  QVERIFY(engine.start());
  QVERIFY(engine.isRunning());

  uint64_t t = 1234567;

  clock.setTimestamp(0);
  clock.advance(t);

  QVERIFY(engine.step()); // m_scheduler.step() -> publish(...)

  QCOMPARE(source->generationCount(), 1u);

  Sample sample;

  QCOMPARE(srv.storage.sample(tags[0]).value, 0.f);

  QCOMPARE(srv.storage.sample(tags[1]).value, 1.f);

  QCOMPARE(srv.storage.sample(tags[2]).value, 2.f);

  QCOMPARE(srv.storage.timestamp(tags[0]), t);

  QCOMPARE(srv.publisherSender.sendCount, 1u);
  QCOMPARE(srv.publisherSender.m_packets.size(), 1);

  PacketReader reader;
  reader.append(srv.publisherSender.m_packets.front().data(),
                srv.publisherSender.m_packets.front().size());

  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::LiveData);

  PublishHeader hdr;
  QVERIFY(reader.read(hdr));

  QCOMPARE(hdr.subscriptionId.value, 1u);
  QCOMPARE(hdr.sequence, 0u);
  QCOMPARE(hdr.timestamp, t);
  QCOMPARE(hdr.valueCount, sub.tags.size());

  std::array<Sample,3> samples;

  QVERIFY(reader.readArray(samples.data(), samples.size()));

  QCOMPARE(samples[0].value, 0.f);
  QCOMPARE(samples[1].value, 1.f);
  QCOMPARE(samples[2].value, 2.f);

  QCOMPARE(reader.remaining(), std::size_t(0));

  engine.stop();
  QVERIFY(!engine.isRunning());
}
*/