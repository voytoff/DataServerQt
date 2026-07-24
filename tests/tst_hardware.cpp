#include "tst_hardware.h"
#include "dataengine.h"
#include "fakeclock.h"
#include "fakelcardmodule.h"
#include "moduledatasource.h"
#include "protocol/publishheader.h"
#include "testsrv.h"
#include <qtestcase.h>

tst_hardware::tst_hardware() { }
tst_hardware::~tst_hardware() = default;

void tst_hardware::test_hardware_schedulerPipeline()
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
  //FakeLCardModule lcmodule;
  auto lcmodule = std::make_unique<FakeLCardModule>();
  auto module = lcmodule.get();
  auto ptr = std::make_unique<ModuleDataSource>(srv.storage, cfg, cfg.modules()[0], std::move(lcmodule), clock);
  //auto source = ptr.get();
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

  QCOMPARE(module->startCalls, 1u);
  QCOMPARE(module->readCalls, 1u);

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

  QCOMPARE(module->stopCalls, 1u);
}
