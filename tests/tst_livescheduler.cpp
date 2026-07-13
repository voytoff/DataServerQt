#include "tst_livescheduler.h"
#include "isender.h"
#include "livescheduler.h"
#include "packetreader.h"
#include "protocol/livedata.h"
#include "publisher.h"
#include "subscription.h"
#include "subscriptionmanager.h"
#include "systemconfiguration.h"
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

