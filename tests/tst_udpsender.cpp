#include "tst_udpsender.h"
#include "udpsender.h"

tst_udpsender::tst_udpsender() { }
tst_udpsender::~tst_udpsender() = default;

void tst_udpsender::test_send()
{
  using namespace qds;

  QUdpSocket receiver;

  QVERIFY(
    receiver.bind(
      QHostAddress::LocalHost,
      0));

  const quint16 port =
    receiver.localPort();

  UdpSender sender;

  Endpoint ep;
  ep.address = "127.0.0.1";
  ep.port = port;

  const std::array<std::byte, 4> data =
    {
      std::byte{1},
      std::byte{2},
      std::byte{3},
      std::byte{4}
    };

  QVERIFY(
    sender.send(
      ep,
      data.data(),
      data.size()));

  QVERIFY(
    receiver.waitForReadyRead(
      1000));

  QCOMPARE(
    receiver.pendingDatagramSize(),
    qint64(data.size()));

  QByteArray buffer;
  buffer.resize(
    receiver.pendingDatagramSize());

  QHostAddress senderAddress;
  quint16 senderPort = 0;

  const qint64 size =
    receiver.readDatagram(
      buffer.data(),
      buffer.size(),
      &senderAddress,
      &senderPort);

  QCOMPARE(
    size,
    qint64(data.size()));

  QCOMPARE(
    static_cast<uint8_t>(buffer[0]),
    uint8_t(1));

  QCOMPARE(
    static_cast<uint8_t>(buffer[1]),
    uint8_t(2));

  QCOMPARE(
    static_cast<uint8_t>(buffer[2]),
    uint8_t(3));

  QCOMPARE(
    static_cast<uint8_t>(buffer[3]),
    uint8_t(4));
}

void tst_udpsender::test_send_bad_address()
{
  using namespace qds;

  UdpSender sender;

  Endpoint ep;
  ep.address = "abc.def.xyz";
  ep.port = 12345;

  std::byte b{0};

  QVERIFY(
    !sender.send(
      ep,
      &b,
      1));
}

void tst_udpsender::test_send_empty()
{
  using namespace qds;

  UdpSender sender;

  Endpoint ep;
  ep.address = "127.0.0.1";
  ep.port = 12345;

  std::byte b{};

  QVERIFY(
    !sender.send(
      ep,
      &b,
      0));
}



