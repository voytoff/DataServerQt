#include "tst_subscriptions.h"
#include "subscription.h"
#include "subscriptionmanager.h"

tst_subscriptions::tst_subscriptions() { }
tst_subscriptions::~tst_subscriptions() = default;


void tst_subscriptions::test_subscriptions_add_remove() {
  qds::SubscriptionManager manager;

  qds::Subscription s1;
  s1.endpoint.address = "127.0.0.1";
  s1.endpoint.port = 5000;
  s1.rate = qds::PublishRate::Hz10;
  s1.tags = { {1}, {2}, {3} };

  auto id = manager.add(s1);

  QVERIFY(id.value != 0);

  QCOMPARE(manager.subscriptions().size(), size_t(1));

  auto* s = manager.find(id);

  QVERIFY(s != nullptr);

  QCOMPARE(s->rate, qds::PublishRate::Hz10);
  QCOMPARE(s->tags.size(), size_t(3));

  QVERIFY(manager.remove(id));

  QCOMPARE(manager.subscriptions().size(), size_t(0));

  QVERIFY(!manager.remove(id));
}

void tst_subscriptions::test_subscriptions_unique_ids()
{
  qds::SubscriptionManager manager;

  qds::Subscription s;

  auto id1 = manager.add(s);
  auto id2 = manager.add(s);
  auto id3 = manager.add(s);

  QVERIFY(id1 != id2);
  QVERIFY(id2 != id3);
  QVERIFY(id1 != id3);
}

void tst_subscriptions::test_subscriptions_id_not_reused()
{
  qds::SubscriptionManager manager;

  qds::Subscription s;

  auto id1 = manager.add(s);
  auto id2 = manager.add(s);

  auto p = manager.remove(id1);
  QVERIFY(p);

  auto id3 = manager.add(s);

  QVERIFY(id3.value > id2.value);
}
