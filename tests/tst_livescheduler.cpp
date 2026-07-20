#include "tst_livescheduler.h"
#include "isender.h"
#include "livescheduler.h"
#include "packetreader.h"
#include "protocol/livedata.h"
#include "protocol/publishheader.h"
#include "publisher.h"
#include "subscription.h"
#include "subscriptionmanager.h"
#include "systemconfiguration.h"
#include "testsrv.h"
#include <qtestcase.h>

tst_livescheduler::tst_livescheduler() { }
tst_livescheduler::~tst_livescheduler() = default;

void tst_livescheduler::test_scheduler_100Hz()
{
  using namespace qds;

  SubscriptionManager manager;
  Publisher publisher;

  SystemConfiguration cfg;
  LiveStorage storage(cfg);
  TestSender sender;

  LiveScheduler scheduler(
    storage,
    manager,
    publisher,
    sender);

  Subscription sub;
  sub.rate = PublishRate::Hz100;

  auto id = manager.add(sub);

  scheduler.addSubscription(
    id,
    PublishRate::Hz100);

  scheduler.tick();

  QCOMPARE(
    sender.sendCount,
    1u);
}

void tst_livescheduler::test_scheduler_10Hz()
{
  using namespace qds;

  SubscriptionManager manager;
  Publisher publisher;

  SystemConfiguration cfg;
  LiveStorage storage(cfg);
  TestSender sender;

  LiveScheduler scheduler(
    storage,
    manager,
    publisher,
    sender);

  Subscription sub;
  sub.rate = PublishRate::Hz10;

  auto id = manager.add(sub);

  scheduler.addSubscription(
    id,
    PublishRate::Hz10);

  for (int i = 0; i < 9; ++i)
    scheduler.tick();

  QCOMPARE(
    sender.sendCount,
    1u);
}

void tst_livescheduler::test_scheduler_1Hz()
{
  using namespace qds;

  SubscriptionManager manager;
  Publisher publisher;

  SystemConfiguration cfg;
  LiveStorage storage(cfg);
  TestSender sender;

  LiveScheduler scheduler(
    storage,
    manager,
    publisher,
    sender);

  Subscription sub;
  sub.rate = PublishRate::Hz1;

  auto id = manager.add(sub);

  scheduler.addSubscription(
    id,
    PublishRate::Hz1);

  for (int i = 0; i < 99; ++i)
    scheduler.tick();

  QCOMPARE(
    sender.sendCount,
    1u);
}

void tst_livescheduler::test_scheduler_100Hz_remove_sub() {
  using namespace qds;

  SubscriptionManager manager;
  Publisher publisher;

  SystemConfiguration cfg;
  LiveStorage storage(cfg);
  TestSender sender;

  LiveScheduler scheduler(
    storage,
    manager,
    publisher,
    sender);

  Subscription sub;
  sub.rate = PublishRate::Hz100;

  auto id = manager.add(sub);

  scheduler.addSubscription(
    id,
    PublishRate::Hz100);

  scheduler.tick();

  QCOMPARE(
    sender.sendCount,
    1u);

  scheduler.removeSubscription(id);

  scheduler.tick();

  QCOMPARE(
    sender.sendCount,
    1u);
}

void tst_livescheduler::test_scheduler_AllHz()
{
  using namespace qds;

  SubscriptionManager manager;
  Publisher publisher;

  SystemConfiguration cfg;
  LiveStorage storage(cfg);
  TestSender sender;

  LiveScheduler scheduler(
    storage,
    manager,
    publisher,
    sender);

  Subscription sub100;
  sub100.rate = PublishRate::Hz100;

  auto id = manager.add(sub100);

  scheduler.addSubscription(
    id,
    PublishRate::Hz100);

  Subscription sub10;
  sub10.rate = PublishRate::Hz10;

  id = manager.add(sub10);

  scheduler.addSubscription(
    id,
    PublishRate::Hz10);

  Subscription sub1;
  sub1.rate = PublishRate::Hz1;

  id = manager.add(sub1);

  scheduler.addSubscription(
    id,
    PublishRate::Hz1);

  scheduler.tick();

  QCOMPARE(
    sender.sendCount,
    3u);
}

void tst_livescheduler::test_scheduler_100Hz_period() {
  using namespace qds;

  SubscriptionManager manager;
  Publisher publisher;

  SystemConfiguration cfg;
  LiveStorage storage(cfg);
  TestSender sender;

  LiveScheduler scheduler(
    storage,
    manager,
    publisher,
    sender);

  Subscription sub;
  sub.rate = PublishRate::Hz100;

  auto id = manager.add(sub);

  scheduler.addSubscription(
    id,
    PublishRate::Hz100);

  scheduler.tick();

  QCOMPARE(
    sender.sendCount,
    1u);

  scheduler.tick();

  QCOMPARE(
    sender.sendCount,
    2u);

}

void tst_livescheduler::test_scheduler_10Hz_period() {
  using namespace qds;

  SubscriptionManager manager;
  Publisher publisher;

  SystemConfiguration cfg;
  LiveStorage storage(cfg);
  TestSender sender;

  LiveScheduler scheduler(
    storage,
    manager,
    publisher,
    sender);

  Subscription sub;
  sub.rate = PublishRate::Hz10;

  auto id = manager.add(sub);

  scheduler.addSubscription(
    id,
    PublishRate::Hz10);

  scheduler.tick();

  QCOMPARE(
    sender.sendCount,
    1u);

  for (int i = 0; i < 10; ++i)
    scheduler.tick();

  QCOMPARE(
    sender.sendCount,
    2u);
}

void tst_livescheduler::test_scheduler_1Hz_period() {
  using namespace qds;

  SubscriptionManager manager;
  Publisher publisher;

  SystemConfiguration cfg;
  LiveStorage storage(cfg);
  TestSender sender;

  LiveScheduler scheduler(
    storage,
    manager,
    publisher,
    sender);

  Subscription sub;
  sub.rate = PublishRate::Hz1;

  auto id = manager.add(sub);

  scheduler.addSubscription(
    id,
    PublishRate::Hz1);

  scheduler.tick();

  QCOMPARE(
    sender.sendCount,
    1u);

  for (int i = 0; i < 100; ++i)
    scheduler.tick();

  QCOMPARE(
    sender.sendCount,
    2u);

}

void tst_livescheduler::test_scheduler_send_subscription_sequence() {
  using namespace qds;

  SubscriptionManager manager;
  Publisher publisher;

  SystemConfiguration cfg;
  LiveStorage storage(cfg);
  TestSender sender;

  LiveScheduler scheduler(
    storage,
    manager,
    publisher,
    sender);

  Subscription sub;
  sub.rate = PublishRate::Hz10;

  auto id = manager.add(sub);

  scheduler.addSubscription(
    id,
    PublishRate::Hz10);

  for (int i = 0; i < 99; ++i)
    scheduler.tick();

  QCOMPARE(
    sender.sendCount,
    10u);

  const Subscription* s = manager.find(id);
  QVERIFY(s != nullptr);

  QCOMPARE(
    s->sequence,
    10u);

  PacketReader reader;

  reader.append(sender.packets[0].data(),
                sender.packets[0].size());

  QVERIFY(reader.nextPacket());

  LiveDataHeader hdr;
  QVERIFY(reader.read(hdr));

  QCOMPARE(hdr.sequence, 0u);

  reader.clear();

  reader.append(sender.packets.back().data(),
                sender.packets.back().size());

  QVERIFY(reader.nextPacket());

  QVERIFY(reader.read(hdr));

  QCOMPARE(hdr.sequence, 9u);
}

void tst_livescheduler::test_scheduler_send_subscriptions_sequence2() {
  using namespace qds;

  SubscriptionManager manager;
  Publisher publisher;

  SystemConfiguration cfg;
  LiveStorage storage(cfg);
  TestSender sender;

  LiveScheduler scheduler(
    storage,
    manager,
    publisher,
    sender);

  Subscription sub1;
  sub1.rate = PublishRate::Hz10;

  auto id1 = manager.add(sub1);

  scheduler.addSubscription(
    id1,
    PublishRate::Hz10);

  Subscription sub2;
  sub2.rate = PublishRate::Hz10;

  auto id2 = manager.add(sub2);

  scheduler.addSubscription(
    id2,
    PublishRate::Hz10);

  for (int i = 0; i < 49; ++i)
    scheduler.tick();

  QCOMPARE(sender.sendCount, 5+5);

  const Subscription* s1 = manager.find(id1);
  QVERIFY(s1 != nullptr);
  QCOMPARE(s1->sequence, 5);

  const Subscription* s2 = manager.find(id2);
  QVERIFY(s2 != nullptr);
  QCOMPARE(s2->sequence, 5);

  QVERIFY(manager.remove(id2));

  for (int i = 0; i < 50; ++i)
    scheduler.tick();

  QCOMPARE(sender.sendCount, 15u);
  const Subscription* checkS1 = manager.find(id1);
  QVERIFY(checkS1 != nullptr);
  QCOMPARE(checkS1->sequence, 10u);

  QCOMPARE(checkS1->sequence, 10u);
  //const Subscription* checkS2 = manager.find(id2);
  //QVERIFY(checkS2 == nullptr);
}

void tst_livescheduler::test_publishFailed_sequenceNotIncremented()
{
  using namespace qds;

  SystemConfiguration cfg;
  SubscriptionManager manager;
  Publisher publisher;

  LiveStorage storage(cfg);
  TestSender sender;

  LiveScheduler scheduler(
    storage,
    manager,
    publisher,
    sender);

  Subscription sub;
  sub.rate = PublishRate::Hz10;
  sub.tags = {{999}};

  auto id = manager.add(sub);

  scheduler.addSubscription(
    id,
    PublishRate::Hz10);

  QCOMPARE(manager.size(), 1u);
  QCOMPARE(id.value, 1u);
  QCOMPARE(manager.find(id)->sequence, 0u);

  scheduler.tick();

  // после неудачной публикации состояние подписки не изменилось.
  QCOMPARE(sender.sendCount, 0u);
  QCOMPARE(manager.find(id)->sequence, 0u);
}

void tst_livescheduler::test_emptySubscription_sequenceIncremented()
{
  using namespace qds;
  SubscriptionManager manager;
  Publisher publisher;

  SystemConfiguration cfg;
  LiveStorage storage(cfg);
  TestSender sender;

  LiveScheduler scheduler(
    storage,
    manager,
    publisher,
    sender);

  Subscription sub;
  sub.rate = PublishRate::Hz10;
  // sub.tags.empty()

  auto id = manager.add(sub);

  scheduler.addSubscription(
    id,
    PublishRate::Hz10);

  QCOMPARE(id.value, 1u);
  QCOMPARE(manager.find(id)->sequence, 0u);

  scheduler.tick();

  // после публикации состояние подписки изменилось.
  QCOMPARE(sender.sendCount, 1u);
  QCOMPARE(manager.find(id)->sequence, 1u);
}

void tst_livescheduler::test_publishPacket_singleTag()
{
  using namespace qds;
  constexpr TagId tags1[] { {0}, {1} };
  SystemConfiguration cfg = createTestConfig(tags1, std::size(tags1));

  TestSrv srv(cfg);

  float expectedValue = 110.0f;
  float values[] = {expectedValue, 201.2f};

  uint64_t time1 = 1234567;
  // модуль 0 обновил данные в livestorage с временем 1234567
  QVERIFY(srv.storage.updateModule(ModuleId{0}, values, time1));


  Subscription sub;
  sub.rate = PublishRate::Hz10;
  sub.tags = {{0}};

  auto id = srv.manager.add(sub);

  srv.scheduler.addSubscription(
    id,
    PublishRate::Hz10);

  QCOMPARE(id.value, 1u);
  QCOMPARE(srv.manager.find(id)->sequence, 0u);

  srv.scheduler.tick();

  // после публикации состояние подписки изменилось.
  QCOMPARE(srv.sender.sendCount, 1u);
  QCOMPARE(srv.manager.find(id)->sequence, 1u);

  PacketReader reader;
  reader.append(srv.sender.packets.front().data(),
                srv.sender.packets.front().size());

  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::LiveData);

  PublishHeader hdr;
  QVERIFY(reader.read(hdr));

  QCOMPARE(hdr.sequence, 0u);
  QCOMPARE(hdr.valueCount, 1u);

  Sample sample;
  QVERIFY(reader.read(sample));

  QCOMPARE(sample.value, expectedValue);

  QCOMPARE(reader.remaining(), 0u);
  QCOMPARE(hdr.timestamp, time1);
}

