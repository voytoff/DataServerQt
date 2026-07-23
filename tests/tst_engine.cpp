#include "tst_engine.h"
#include "dataengine.h"
#include "datasourcemanager.h"
#include "fakeclock.h"
#include "fakedatasource.h"
#include "generatordatasource.h"
#include "protocol/publishheader.h"
#include "testsrv.h"
#include <qtestcase.h>

tst_engine::tst_engine() { }
tst_engine::~tst_engine() = default;

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

  auto ptr = std::make_unique<FakeDataSource>();
  FakeDataSource* src = ptr.get();

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

  clock.set(0);
  clock.advance(t);

  QVERIFY(engine.step());

  QCOMPARE(source->generationCount(), 1u);

  Sample sample;

  QVERIFY(srv.storage.read(tags[0], sample));
  QCOMPARE(sample.value, 0.f);

  QVERIFY(srv.storage.read(tags[1], sample));
  QCOMPARE(sample.value, 1.f);

  QVERIFY(srv.storage.read(tags[2], sample));
  QCOMPARE(sample.value, 2.f);

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

  clock.set(0);
  clock.advance(t);

  QVERIFY(engine.step()); // m_scheduler.step() -> publish(...)

  QCOMPARE(source->generationCount(), 1u);

  Sample sample;

  QVERIFY(srv.storage.read(tags[0], sample));
  QCOMPARE(sample.value, 0.f);

  QVERIFY(srv.storage.read(tags[1], sample));
  QCOMPARE(sample.value, 1.f);

  QVERIFY(srv.storage.read(tags[2], sample));
  QCOMPARE(sample.value, 2.f);

  QCOMPARE(srv.storage.timestamp(tags[0]), t);

  QCOMPARE(srv.sender.sendCount, 1u);
  QCOMPARE(srv.sender.packets.size(), 1);

  PacketReader reader;
  reader.append(srv.sender.packets.front().data(),
                srv.sender.packets.front().size());

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
