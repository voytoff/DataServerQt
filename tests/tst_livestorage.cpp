#include "tst_livestorage.h"
#include "livestorage.h"
#include "packetreader.h"
#include "packetwriter.h"
#include "protocol/livedata.h"
#include "protocol/packettype.h"
#include "protocol/publishheader.h"
#include "publisher.h"
#include "systemconfiguration.h"
#include "taginfo.h"
#include <qtestcase.h>

tst_livestorage::tst_livestorage() { }
tst_livestorage::~tst_livestorage() = default;

void tst_livestorage::test_livestorage()
{
  using namespace qds;
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

  QVERIFY(storage.updateModule(ModuleId{0}, values, 100));

  QCOMPARE(storage.sample(qds::TagId{0}).value, 42.0f);
  QCOMPARE(storage.moduleTimestamp(qds::ModuleId{0}), 100u);
}

void tst_livestorage::test_liveDataHeader()
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

void tst_livestorage::test_LiveDataPacket()
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

void tst_livestorage::test_liveDataPayload()
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

void tst_livestorage::test_publishSamples() {
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
  QVERIFY(storage.updateModule(ModuleId{0}, values, t));

  // 3. Подписка
  Subscription s1;
  s1.endpoint.address = "127.0.0.1";
  s1.endpoint.port = 35015;
  s1.rate = PublishRate::Hz10;
  s1.tags = { {0}, {1} };

  PacketWriter writer{};
  Publisher pub{};

  QVERIFY(pub.publish(storage, s1, s1.sequence, t, writer));

  PacketReader reader;
  reader.append(writer.data(), writer.size());

  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::LiveData);

  PublishHeader ldh;
  QVERIFY(reader.read(ldh));

  QCOMPARE(ldh.subscriptionId.value, 0u);
  QCOMPARE(ldh.sequence, 0u);
  QCOMPARE(ldh.timestamp, t);
  QCOMPARE(ldh.valueCount, 2u);

  std::array<Sample,2> samples{};

  QVERIFY(reader.readArray(samples.data(), samples.size()));

  QCOMPARE(samples[0].value, 100.0f);
  QCOMPARE(samples[1].value, 101.2f);

  QVERIFY(reader.remaining() == 0);
}

void tst_livestorage::test_publishSamples_reverseOrder() {
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
  QVERIFY(storage.updateModule(ModuleId{0}, values, t));

  // 3. Подписка
  Subscription s1;
  s1.endpoint.address = "127.0.0.1";
  s1.endpoint.port = 35015;
  s1.rate = PublishRate::Hz10;
  s1.tags = { {1}, {0} };

  PacketWriter writer{};
  Publisher pub{};

  QVERIFY(pub.publish(storage, s1, s1.sequence, t, writer));

  PacketReader reader;
  reader.append(writer.data(), writer.size());

  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::LiveData);

  PublishHeader ldh;

  QVERIFY(reader.read(ldh));
  QCOMPARE(ldh.subscriptionId.value, 0u);
  QCOMPARE(ldh.sequence, 0u);
  QCOMPARE(ldh.timestamp, t);
  QCOMPARE(ldh.valueCount, 2u);

  std::array<Sample,2> samples{};

  QVERIFY(reader.readArray(samples.data(), samples.size()));

  QCOMPARE(samples[0].value, 101.2f);
  QCOMPARE(samples[1].value, 100.0f);

  QVERIFY(reader.remaining() == 0);
}

void tst_livestorage::test_updateModule_read()
{
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

  TagInfo t3{.tag = {2}, .module = {0}, .channel = {2}};
  cfg.addTag(t3);

  // 2. LiveStorage
  LiveStorage storage(cfg);

  float values[] = {110.0f, 201.2f, 303.33f};

  uint64_t t = 1234567;
  // модуль 0 обновил данные в livestorage с временем 1234567
  QVERIFY(storage.updateModule(ModuleId{0}, values, t));

  // 3. Подписка
  Subscription s1;
  s1.endpoint.address = "127.0.0.1";
  s1.endpoint.port = 35022;
  s1.rate = PublishRate::Hz10;
  s1.tags = { {0}, {1}, {2} };

  PacketWriter writer;
  Publisher pub;

  QVERIFY(pub.publish(storage, s1, s1.sequence, t, writer));

  PacketReader reader;
  reader.append(writer.data(), writer.size());

  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::LiveData);

  PublishHeader ldh;
  QVERIFY(reader.read(ldh));

  QCOMPARE(ldh.subscriptionId.value, 0u);
  QCOMPARE(ldh.sequence, 0u);
  QCOMPARE(ldh.timestamp, t);
  QCOMPARE(ldh.valueCount, s1.tags.size());

  std::array<Sample, 3> samples{};

  QVERIFY(reader.readArray(samples.data(), samples.size()));

  QCOMPARE(samples[0].value, values[0]);
  QCOMPARE(samples[1].value, values[1]);
  QCOMPARE(samples[2].value, values[2]);

  QVERIFY(reader.remaining() == 0);
}

void tst_livestorage::test_updateModule_invalidSize_keepsPreviousValues()
{
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

  TagInfo t3{.tag = {2}, .module = {0}, .channel = {2}};
  cfg.addTag(t3);

  // 2. LiveStorage
  LiveStorage storage(cfg);

  float values[] = {110.0f, 201.2f, 303.33f};

  uint64_t t = 1234567;
  // модуль 0 обновил данные в livestorage с временем 1234567
  QVERIFY(storage.updateModule(ModuleId{0}, values, t));

  // 3. Подписка
  Subscription s1;
  s1.endpoint.address = "127.0.0.1";
  s1.endpoint.port = 35022;
  s1.rate = PublishRate::Hz10;
  s1.tags = { {0}, {1}, {2} };

  PacketWriter writer;
  Publisher pub;

  QVERIFY(pub.publish(storage, s1, s1.sequence, t, writer));

  PacketReader reader;
  reader.append(writer.data(), writer.size());

  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::LiveData);

  PublishHeader ldh;
  QVERIFY(reader.read(ldh));

  QCOMPARE(ldh.subscriptionId.value, 0u);
  QCOMPARE(ldh.sequence, 0u);
  QCOMPARE(ldh.timestamp, t);
  QCOMPARE(ldh.valueCount, s1.tags.size());

  std::array<Sample, 3> samples{};

  QVERIFY(reader.readArray(samples.data(), samples.size()));

  QCOMPARE(samples[0].value, values[0]);
  QCOMPARE(samples[1].value, values[1]);
  QCOMPARE(samples[2].value, values[2]);

  QVERIFY(reader.remaining() == 0);

  t = 7654321;
  float values2[] = {11.1f, 22.2f};
  // модуль 0 обновляет неполные данные в livestorage с временем 7654321
  QVERIFY(!storage.updateModule(ModuleId{0}, values2, t));

  QVERIFY(pub.publish(storage, s1, s1.sequence, t, writer));

  reader.clear();
  reader.append(writer.data(), writer.size());

  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::LiveData);

  QVERIFY(reader.read(ldh));

  QCOMPARE(ldh.subscriptionId.value, 0u);
  QCOMPARE(ldh.sequence, 0u);
  QCOMPARE(ldh.timestamp, t);
  QCOMPARE(ldh.valueCount, s1.tags.size());

  std::array<Sample, 3> samples2{};

  QVERIFY(reader.readArray(samples2.data(), samples2.size()));

  QCOMPARE(samples2[0].value, values[0]);
  QCOMPARE(samples2[1].value, values[1]);
  QCOMPARE(samples2[2].value, values[2]);

  QVERIFY(reader.remaining() == 0);
}

void tst_livestorage::test_read_invalidTag()
{
  using namespace qds;
  // 1. Configuration
  SystemConfiguration cfg;

  ModuleInfo m;
  m.id.value = 0;
  cfg.addModule(m);

  TagInfo t1{.tag = {0}, .module = {0}, .channel = {0}};
  cfg.addTag(t1);
  TagInfo t2{.tag = {1}, .module = {0}, .channel = {1}};
  cfg.addTag(t2);

  // 2. LiveStorage
  LiveStorage storage(cfg);

  float values[] = {110.0f, 0.222f};

  uint64_t t = 1234567;
  // модуль 0 обновил данные в livestorage с временем 1234567
  QVERIFY(storage.updateModule(ModuleId{0}, values, t));

  Sample sample;

  QVERIFY(storage.read(TagId{1}, sample));
  QCOMPARE(sample.value, values[1]);

  QVERIFY(!storage.read(TagId{3}, sample));
  QCOMPARE(sample.value, values[1]);
}

void tst_livestorage::test_updateModule_multipleUpdates()
{
  using namespace qds;
  // 1. Configuration
  SystemConfiguration cfg;

  ModuleInfo m;
  m.id.value = 0;
  cfg.addModule(m);

  TagInfo t1{.tag = {0}, .module = {0}, .channel = {0}};
  cfg.addTag(t1);

  TagInfo t2{.tag = {1}, .module = {0}, .channel = {1}};
  cfg.addTag(t2);

  TagInfo t3{.tag = {2}, .module = {0}, .channel = {2}};
  cfg.addTag(t3);

  // 2. LiveStorage
  LiveStorage storage(cfg);

  float values[] = {110.0f, 201.2f, 303.33f};

  uint64_t time1 = 1234567;
  // модуль 0 обновил данные в livestorage с временем 1234567
  QVERIFY(storage.updateModule(ModuleId{0}, values, time1));

  float values2[] = {101.7f, 221.28f, 330.30f};

  uint64_t time2 = 7654321;
  // модуль 0 обновил данные в livestorage с временем 1234567
  QVERIFY(storage.updateModule(ModuleId{0}, values2, time2));

  // 3. Подписка
  Subscription s1;
  s1.id = {1};
  s1.endpoint.address = "127.0.0.1";
  s1.endpoint.port = 35022;
  s1.rate = PublishRate::Hz10;
  s1.tags = { {0}, {1}, {2} };

  PacketWriter writer;
  Publisher pub;

  QVERIFY(pub.publish(storage, s1, s1.sequence, time2, writer));

  PacketReader reader;
  reader.append(writer.data(), writer.size());

  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::LiveData);

  PublishHeader ldh;
  QVERIFY(reader.read(ldh));

  QCOMPARE(ldh.subscriptionId.value, 1u);
  QCOMPARE(ldh.sequence, 0u);
  QCOMPARE(ldh.timestamp, time2);
  QCOMPARE(ldh.valueCount, s1.tags.size());

  std::array<Sample, 3> samples{};

  QVERIFY(reader.readArray(samples.data(), samples.size()));

  QCOMPARE(samples[0].value, values2[0]);
  QCOMPARE(samples[1].value, values2[1]);
  QCOMPARE(samples[2].value, values2[2]);

  QVERIFY(reader.remaining() == 0);
}
