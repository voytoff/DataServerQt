#include "tst_publisher.h"
#include "livestorage.h"
#include "moduleinfo.h"
#include "packetreader.h"
#include "packetwriter.h"
#include "protocol/publishheader.h"
#include "publisher.h"
#include "subscription.h"
#include "systemconfiguration.h"
#include "taginfo.h"
#include "testsrv.h"
#include <qtestcase.h>

tst_publisher::tst_publisher() { }
tst_publisher::~tst_publisher() = default;

/*
static qds::LiveStorage createLiveStorage() {
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m;
  m.id.value = 0;
  cfg.addModule(m);

  TagInfo t1;
  t1.tag.value = 0;
  t1.module.value = 0;
  t1.channel.value = 0;
  cfg.addTag(t1);

  TagInfo t2;
  t2.tag.value = 1;
  t2.module.value = 0;
  t2.channel.value = 1;
  cfg.addTag(t2);

  LiveStorage storage(cfg);

  return storage;
}

void tst_publisher::test_publish_sequence()
{
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m;
  m.id.value = 0;
  cfg.addModule(m);

  TagInfo t1;
  t1.tag.value = 0;
  t1.module.value = 0;
  t1.channel.value = 0;
  cfg.addTag(t1);

  TagInfo t2;
  t2.tag.value = 1;
  t2.module.value = 0;
  t2.channel.value = 1;
  cfg.addTag(t2);

  LiveStorage storage(cfg);

  float values[] = {1.1f, 2.2f};

  uint64_t t = 1234321;
  // модуль 0 обновил данные в livestorage с временем 1234321
  QVERIFY(storage.updateModule(ModuleId{0}, values, t));

  // Подписка
  Subscription s1;
  s1.endpoint.address = "127.0.0.1";
  s1.endpoint.port = 35015;
  s1.rate = PublishRate::Hz10;
  s1.tags = { {0}, {1} };

  PacketWriter writer{};
  Publisher pub{};

  pub.publish(storage, s1, 10u, writer);

  PacketReader reader;
  reader.append(writer.data(), writer.size());

  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::LiveData);

  PublishHeader ldh;
  QVERIFY(reader.read(ldh));

  QCOMPARE(ldh.subscriptionId.value, 0u);
  QCOMPARE(ldh.sequence, 10u);
  QCOMPARE(ldh.timestamp, t);
  QCOMPARE(ldh.valueCount, 2u);

  std::array<Sample,2> samples{};

  QVERIFY(reader.readArray(samples.data(), samples.size()));

  QCOMPARE(samples[0].value, 1.1f);
  QCOMPARE(samples[1].value, 2.2f);

  QVERIFY(reader.remaining() == 0);
}

void tst_publisher::test_publish_emptySubscription()
{
  using namespace qds;

  SystemConfiguration cfg;

  ModuleInfo m;
  m.id.value = 0;
  cfg.addModule(m);

  LiveStorage storage(cfg);

  Subscription sub;
  sub.id.value = 1;
  sub.rate = PublishRate::Hz10;

  // tags пустой

  PacketWriter writer;
  Publisher publisher;

  QVERIFY(
    publisher.publish(
      storage,
      sub,
      5u,
      writer));

  PacketReader reader;

  reader.append(
    writer.data(),
    writer.size());

  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::LiveData);

  PublishHeader hdr;

  QVERIFY(reader.read(hdr));

  QCOMPARE(hdr.subscriptionId.value, 1u);
  QCOMPARE(hdr.sequence, 5u);
  QCOMPARE(hdr.timestamp, 0u);
  QCOMPARE(hdr.valueCount, 0u);

  QVERIFY(reader.remaining() == 0);
}
*/
void tst_publisher::test_publish_reuseWriter()
{
  using namespace qds;
  SystemConfiguration cfg  = createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  TestPublisherSender sender;
  SubscriptionManager subscriptions;

  Publisher publisher(layout, subscriptions, sender, 1000);

  // подписка на 3 сигнала (2 raw, 1 calc)
  Subscription sub
  {
    .endpoint = {
      .address = "127.0.0.1",
      .port = 5000
    },
    .signalIds = {
      SignalId{0},
      SignalId{1},
      SignalId{23}
    },
    .rate = PublishRate::Hz10,
  };

  QVERIFY(subscriptions.add(sub));

  Frame frame;
  frame.initialize(layout);

  frame.number = FrameNumber{1};
  frame.timestamp = Timestamp{100};
  frame.wallTime = WallClockTime{200};

  frame.raw().setValue(0, 10.0);
  frame.raw().setValue(1, 20.0);

  frame.calculated().setValue(0, 30.0);
  frame.calculated().setValue(1, 40.0);
  frame.calculated().setValue(2, 50.0);

  publisher.publish(frame);

  QCOMPARE(sender.sendCount, 1);

  PacketReader reader;
  reader.append(
    sender.m_packets[0].data(),
    sender.m_packets[0].size());

  QVERIFY(reader.nextPacket());

  PublishHeader hdr;

  QVERIFY(reader.read(hdr));

  QCOMPARE(hdr.subscriptionId, SubscriptionId{1u});
  QCOMPARE(hdr.valueCount, 3);
  QCOMPARE(hdr.timestamp, 100u);
  QCOMPARE(hdr.sequence, 1);

  std::array<double, 3> values;
  QVERIFY(reader.readArray(values.data(), values.size()));

  QCOMPARE(values[0], 10.0);
  QCOMPARE(values[1], 20.0);
  QCOMPARE(values[2], 50.0);

  QVERIFY(reader.remaining() == 0);


  publisher.publish(frame);

  QCOMPARE(sender.sendCount, 2);

  PacketReader reader2;

  reader2.append(
    sender.m_packets[1].data(),
    sender.m_packets[1].size());

  QVERIFY(reader2.nextPacket());

  PublishHeader hdr2;

  QVERIFY(reader2.read(hdr2));

  QCOMPARE(
    hdr2.subscriptionId,
    SubscriptionId{1u});

  QCOMPARE(
    hdr2.sequence,
    2u);
}

void tst_publisher::test_publish_failSignal()
{
  using namespace qds;
  SystemConfiguration cfg  = createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  TestPublisherSender sender;
  SubscriptionManager subscriptions;

  // подписка на 3 сигнала (2 raw, 1 calc)
  Subscription sub
    {
     .endpoint = {
       .address = "127.0.0.1",
       .port = 5000
     },
     .signalIds = {
       SignalId{0},
       SignalId{1},
       SignalId{23}
     },
     .rate = PublishRate::Hz10,
     };

  QVERIFY(subscriptions.add(sub));

  Frame frame;
  frame.initialize(layout);

  frame.number = FrameNumber{1};
  frame.timestamp = Timestamp{100};
  frame.wallTime = WallClockTime{200};

  frame.raw().setValue(0, 10.0);
  frame.raw().setValue(1, 20.0);

  frame.calculated().setValue(0, 30.0);
  frame.calculated().setValue(1, 40.0);
  frame.calculated().setValue(2, 50.0);

  Publisher publisher(layout, subscriptions, sender, 1000);

  publisher.publish(frame);

  QCOMPARE(sender.sendCount, 1);

  PacketReader reader;
  reader.append(
    sender.m_packets[0].data(),
    sender.m_packets[0].size());

  QVERIFY(reader.nextPacket());

  PublishHeader hdr;

  QVERIFY(reader.read(hdr));

  QCOMPARE(hdr.subscriptionId, SubscriptionId{1u});
  QCOMPARE(hdr.valueCount, 3);
  QCOMPARE(hdr.timestamp, 100u);
  QCOMPARE(hdr.sequence, 1);

  std::array<double, 3> values;
  QVERIFY(reader.readArray(values.data(), values.size()));

  QCOMPARE(values[0], 10.0);
  QCOMPARE(values[1], 20.0);
  QCOMPARE(values[2], 50.0);

  QVERIFY(reader.remaining() == 0);

  auto s = subscriptions.find(SubscriptionId{1});
  s->signalIds[2].value = 24;

  publisher.publish(frame);

  QCOMPARE(sender.sendCount, 1);
  QCOMPARE(sender.m_packets.size(), 1);

  s = subscriptions.find(SubscriptionId{1});

  QVERIFY(s != nullptr);

  QCOMPARE(
    s->sequence,
    1u);

  s->signalIds[2] = SignalId{23};

  publisher.publish(frame);

  QCOMPARE(sender.sendCount, 2u);

  PacketReader reader2;

  reader2.append(
    sender.m_packets[1].data(),
    sender.m_packets[1].size());

  QVERIFY(reader2.nextPacket());

  PublishHeader hdr2;

  QVERIFY(reader2.read(hdr2));

  QCOMPARE(hdr2.sequence, 2u);
}

void tst_publisher::test_publish_publishRate()
{
  using namespace qds;
  SystemConfiguration cfg  = createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  TestPublisherSender sender;
  SubscriptionManager subscriptions;

  Subscription sub0
    {
     .endpoint = {
       .address = "127.0.0.1",
       .port = 5000
     },
     .signalIds = {
       SignalId{17}
     },
     .rate = PublishRate::Hz100,
     };
  QVERIFY(subscriptions.add(sub0));

  Subscription sub1
    {
     .endpoint = {
       .address = "127.0.0.1",
       .port = 5001
     },
     .signalIds = {
       SignalId{4}
     },
     .rate = PublishRate::Hz10,
     };
  QVERIFY(subscriptions.add(sub1));

  Subscription sub2
    {
     .endpoint = {
       .address = "127.0.0.1",
       .port = 35000
     },
     .signalIds = {
       SignalId{23}
     },
     .rate = PublishRate::Hz1,
     };
  QVERIFY(subscriptions.add(sub2));

  Frame frame;
  frame.initialize(layout);

  frame.number = FrameNumber{1};
  frame.timestamp = Timestamp{100};
  frame.wallTime = WallClockTime{200};

  frame.calculated().setValue(0, 30.0);
  frame.calculated().setValue(1, 40.0);
  frame.calculated().setValue(2, 50.0);

  Publisher publisher(layout, subscriptions, sender, 1000);

  for (int i = 1; i <= 2000; ++i) {
    frame.number = FrameNumber{static_cast<uint64_t>(i)};
    frame.timestamp = Timestamp{static_cast<uint64_t>(i*100)};
    frame.wallTime = WallClockTime{i*200};

    publisher.publish(frame);
  }

  QCOMPARE(sender.sendCount, 200 + 20 + 2);

  PacketReader reader;

  int n1 = 0;
  int n10 = 0;
  int n100 = 0;

  for (int n = 0; n < 222; ++n)
  {
    reader.clear();
    reader.append(
      sender.m_packets[n].data(),
      sender.m_packets[n].size());

    QVERIFY(reader.nextPacket());

    PublishHeader hdr;
    QVERIFY(reader.read(hdr));

    Sample sample;
    QVERIFY(reader.read(sample));

    QVERIFY(reader.remaining() == 0);

    if (hdr.subscriptionId == SubscriptionId{1})
    {
      QCOMPARE(sample.value, 30.0);
      QCOMPARE(
        hdr.sequence,
        static_cast<uint32_t>(n100 + 1));
      ++n100;
    }
    else if (hdr.subscriptionId == SubscriptionId{2})
    {
      QCOMPARE(sample.value, 40.0);
      QCOMPARE(
        hdr.sequence,
        static_cast<uint32_t>(n10 + 1));
      ++n10;
    }
    else if (hdr.subscriptionId == SubscriptionId{3})
    {
      QCOMPARE(sample.value, 50.0);
      QCOMPARE(
        hdr.sequence,
        static_cast<uint32_t>(n1 + 1));
      ++n1;
    }
    else
    {
      QFAIL("Unexpected subscription ID");
    }
  }
  QCOMPARE(n100, 200);
  QCOMPARE(n10, 20);
  QCOMPARE(n1, 2);
}
