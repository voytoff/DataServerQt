#include "test_dataprotocol.h"
#include "datatypes.h"
#include "packetreader.h"
#include "packetwriter.h"
#include "protocol/subscriptionpackets.h"
#include <qtestcase.h>

test_dataprotocol::test_dataprotocol() { }
test_dataprotocol::~test_dataprotocol() = default;

/*
1. PacketWriter
2. writer.begin(PacketType)
3. writer.write(...)
4. writer.writeArray(...)
5. PacketReader
6. reader.nextPacket()
7. reader.read(...)
8. reader.readArray(...)
9. reader.eof()
*/
void test_dataprotocol::test_subscribeListPacket()
{
  using namespace qds;
  PacketWriter writer;

  writer.begin(qds::PacketType::SubscribeList);

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.tagCount = 3;

  writer.write(req);

  constexpr TagId tags[]
    {
      {5},
      {10},
      {100}
    };

  // Далее в пакете располагается массив TagId
  writer.writeArray(tags, std::size(tags));

  PacketReader reader;
  reader.append(writer.data(), writer.size());

  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::SubscribeList);

  SubscribeListRequest testReq;
  QVERIFY(reader.read(testReq));

  QCOMPARE(testReq.rate, PublishRate::Hz10);
  QCOMPARE(testReq.tagCount, std::size(tags));

  TagId testTags[std::size(tags)];
  QVERIFY(reader.readArray(testTags, std::size(testTags)));

  QCOMPARE(testTags[0], TagId{5});
  QCOMPARE(testTags[1], TagId{10});
  QCOMPARE(testTags[2], TagId{100});

  QVERIFY(reader.eof());
}

void test_dataprotocol::test_subscribeResponsePacket()
{

}
