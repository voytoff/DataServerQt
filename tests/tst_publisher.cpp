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
#include <qtestcase.h>

tst_publisher::tst_publisher() { }
tst_publisher::~tst_publisher() = default;

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

void tst_publisher::test_publish_invalidTag()
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

  // 3. Подписка
  Subscription s1;
  s1.endpoint.address = "127.0.0.1";
  s1.endpoint.port = 35015;
  s1.rate = PublishRate::Hz10;
  s1.tags = { {0}, {2} }; // 2 тег отсутствует в LiveStorage

  PacketWriter writer;
  Publisher pub;

  QVERIFY(!pub.publish(storage, s1, t, writer));
  QCOMPARE(writer.size(), std::size_t(0));
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

void tst_publisher::test_publish_reuseWriter()
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
  QVERIFY(storage.updateModule(ModuleId{0}, values, 1234321));

  PacketWriter writer;
  Publisher pub;

  // Подписки
  // подписка на два тега
  Subscription s1;
  s1.endpoint.address = "127.0.0.1";
  s1.endpoint.port = 35015;
  s1.rate = PublishRate::Hz10;
  s1.tags = { {0}, {1} };

  pub.publish(storage, s1, t, writer);

  auto size1 = writer.size();

  // подписка на один тег
  Subscription s2;
  s2.endpoint.address = "127.0.0.1";
  s2.endpoint.port = 35016;
  s2.rate = PublishRate::Hz10;
  s2.tags = { {0} };

  pub.publish(storage, s2, t, writer);

  auto size2 = writer.size();

  QVERIFY(size2 < size1);

  PacketReader reader;

  reader.append(
    writer.data(),
    writer.size());

  QVERIFY(reader.nextPacket());

  PublishHeader hdr;

  QVERIFY(reader.read(hdr));

  QCOMPARE(hdr.subscriptionId, s2.id);
  QCOMPARE(hdr.valueCount, 1u);

  Sample sample;

  QVERIFY(reader.read(sample));

  QCOMPARE(sample.value, 1.1f);

  QVERIFY(reader.remaining() == 0);
}
