#include <QTest>

#include "datatypes.h"
#include "livestorage.h"
#include "packetreader.h"
#include "packetwriter.h"
#include "publisher.h"
#include "subscriptionmanager.h"
#include "systemconfiguration.h"
#include "protocol/livedata.h"

class test_common : public QObject
{
  Q_OBJECT

public:
  test_common();
  ~test_common() override;

private slots:
  void test_configuration_moduleTags();
  void test_livestorage();
  void test_subscriptions_add_remove();
  void test_subscriptions_unique_ids();
  void test_subscriptions_id_not_reused();
  void test_liveDataHeader();
  void test_LiveDataPacket();
  void test_liveDataPayload();
  void test_livePublisher();
  void test_livePublisher_reverse();
};

test_common::test_common() { }

test_common::~test_common() = default;

void test_common::test_configuration_moduleTags()
{
  using namespace qds;

  SystemConfiguration cfg;

  ModuleInfo m0;
  m0.id = {0};
  cfg.addModule(m0);

  TagInfo t0;
  t0.tag = {0};
  t0.module = {0};
  cfg.addTag(t0);

  TagInfo t5;
  t5.tag = {5};
  t5.module = {0};
  cfg.addTag(t5);

  const auto& tags = cfg.moduleTags(ModuleId{0});

  QCOMPARE(tags.size(), 2u);
  QCOMPARE(tags[0].value, 0u);
  QCOMPARE(tags[1].value, 5u);
}

void test_common::test_livestorage()
{
  qds::SystemConfiguration cfg;

  qds::ModuleInfo m;
  m.id.value = 0;
  cfg.addModule(m);

  qds::TagInfo t;
  t.tag.value = 0;
  t.module.value = 0;
  t.channel.value = 0;
  cfg.addTag(t);

  qds::LiveStorage storage(cfg);

  float values[] = {42.0f};

  storage.updateModule(qds::ModuleId{0}, values, 100);

  QCOMPARE(storage.sample(qds::TagId{0}).value, 42.0f);
  QCOMPARE(storage.moduleTimestamp(qds::ModuleId{0}), 100u);
}

void test_common::test_subscriptions_add_remove() {
  qds::SubscriptionManager manager;

  qds::Subscription s1;
  s1.endpoint.address = "127.0.0.1";
  s1.endpoint.port = 5000;
  s1.periodMs = 100;
  s1.tags = { {1}, {2}, {3} };

  auto id = manager.add(s1);

  QVERIFY(id.value != 0);

  QCOMPARE(manager.subscriptions().size(), size_t(1));

  auto* s = manager.find(id);

  QVERIFY(s != nullptr);

  QCOMPARE(s->periodMs, 100u);
  QCOMPARE(s->tags.size(), size_t(3));

  QVERIFY(manager.remove(id));

  QCOMPARE(manager.subscriptions().size(), size_t(0));

  QVERIFY(!manager.remove(id));
}

void test_common::test_subscriptions_unique_ids()
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

void test_common::test_subscriptions_id_not_reused()
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


void test_common::test_liveDataHeader()
{
  using namespace qds;
  LiveDataHeader hdr;

  hdr.subscriptionId = SubscriptionId{123};
  hdr.sequence = 456;
  hdr.timestamp = 789;
  hdr.sampleCount = 10;

  PacketWriter writer;

  writer.begin(PacketType::LiveData);
  writer.write(hdr);

  PacketReader reader;

  reader.append(writer.data(), writer.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(reader.header().type, PacketType::LiveData);

  LiveDataHeader hdr2;
  QVERIFY(reader.read(hdr2));

  QCOMPARE(hdr2.subscriptionId.value, 123u);
  QCOMPARE(hdr2.sequence, 456u);
  QCOMPARE(hdr2.timestamp, 789ull);
  QCOMPARE(hdr2.sampleCount, 10u);
}

void test_common::test_LiveDataPacket()
{
  using namespace qds;
  PacketWriter writer;

  writer.begin(PacketType::LiveData);

  LiveDataHeader out;

  out.subscriptionId.value = 42;
  out.sequence = 100;
  out.timestamp = 123456;
  out.sampleCount = 3;

  writer.write(out);

  float values[] = {1.0f, 2.0f, 3.0f};

  writer.writeArray(values, 3);


  PacketReader reader;

  reader.append(writer.data(), writer.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(reader.header().type, PacketType::LiveData);


  LiveDataHeader in;

  QVERIFY(reader.read(in));

  QCOMPARE(in.subscriptionId.value, 42u);
  QCOMPARE(in.sequence, 100u);
  QCOMPARE(in.timestamp, 123456ull);
  QCOMPARE(in.sampleCount, 3u);


  float result[3]{};

  QVERIFY(reader.readArray(result, 3));

  QCOMPARE(result[0], 1.0f);
  QCOMPARE(result[1], 2.0f);
  QCOMPARE(result[2], 3.0f);
}

void test_common::test_liveDataPayload()
{
  using namespace qds;
  LiveDataHeader hdr;

  hdr.subscriptionId.value = 10;
  hdr.sequence = 5;
  hdr.timestamp = 123456789;
  hdr.sampleCount = 3;

  float values[] =
    {
      1.1F,
      2.2F,
      3.3F
    };

  PacketWriter writer;
  writer.begin(PacketType::LiveData);

  writer.write(hdr);
  writer.writeArray(values, 3);

  LiveDataHeader readHdr;

  PacketReader reader;
  reader.append(writer.data(), writer.size());

  QVERIFY(reader.nextPacket());

  reader.read(readHdr);

  std::array<float,3> readValues{};

  reader.readArray(readValues.data(),
                   readValues.size());

  QCOMPARE(readValues[0], 1.1f);
  QCOMPARE(readValues[1], 2.2f);
  QCOMPARE(readValues[2], 3.3f);
}


void test_common::test_livePublisher() {
  using namespace qds;
  // 1. Configuration
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

  // 2. LiveStorage
  LiveStorage storage(cfg);

  float values[] = {100.0f, 101.2f};

  uint64_t t = 1234567;
  // модуль 0 обновил данные в livestorage с временем 1234567
  storage.updateModule(ModuleId{0}, values, t);

  // 3. Подписка
  Subscription s1;
  s1.endpoint.address = "127.0.0.1";
  s1.endpoint.port = 35015;
  s1.periodMs = 100;
  s1.tags = { {0}, {1} };

  PacketWriter writer{};
  Publisher pub{};

  pub.publish(storage, s1, 0, t, writer);

  PacketReader reader;
  reader.append(writer.data(), writer.size());

  QVERIFY(reader.nextPacket());
  LiveDataHeader ldh;

  QVERIFY(reader.read(ldh));
  QCOMPARE(ldh.subscriptionId.value, 0u);
  QCOMPARE(ldh.sequence, 0u);
  QCOMPARE(ldh.timestamp, t);
  QCOMPARE(ldh.sampleCount, 2u);

  QCOMPARE(reader.packetType(), PacketType::LiveData);

  std::array<float,2> pubValues{};

  QVERIFY(reader.readArray(pubValues.data(),
                           pubValues.size()));

  QCOMPARE(pubValues[0], 100.0f);
  QCOMPARE(pubValues[1], 101.2f);

  QVERIFY(reader.eof());
}

void test_common::test_livePublisher_reverse() {
  using namespace qds;
  // 1. Configuration
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

  // 2. LiveStorage
  LiveStorage storage(cfg);

  float values[] = {100.0f, 101.2f};

  uint64_t t = 1234567;
  // модуль 0 обновил данные в livestorage с временем 1234567
  storage.updateModule(ModuleId{0}, values, t);

  // 3. Подписка
  Subscription s1;
  s1.endpoint.address = "127.0.0.1";
  s1.endpoint.port = 35015;
  s1.periodMs = 100;
  s1.tags = { {1}, {0} };

  PacketWriter writer{};
  Publisher pub{};

  pub.publish(storage, s1, 0, t, writer);

  PacketReader reader;
  reader.append(writer.data(), writer.size());

  QVERIFY(reader.nextPacket());
  LiveDataHeader ldh;

  QVERIFY(reader.read(ldh));
  QCOMPARE(ldh.subscriptionId.value, 0u);
  QCOMPARE(ldh.sequence, 0u);
  QCOMPARE(ldh.timestamp, t);
  QCOMPARE(ldh.sampleCount, 2u);

  QCOMPARE(reader.packetType(), PacketType::LiveData);

  std::array<float,2> pubValues{};

  QVERIFY(reader.readArray(pubValues.data(),
                           pubValues.size()));

  QCOMPARE(pubValues[1], 100.0f);
  QCOMPARE(pubValues[0], 101.2f);

  QVERIFY(reader.eof());
}




QTEST_APPLESS_MAIN(test_common)

#include "tst_test_common.moc"
