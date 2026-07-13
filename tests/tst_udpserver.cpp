#include "tst_udpserver.h"
#include "isender.h"
#include "livescheduler.h"
#include "publisher.h"
#include "subscriptionmanager.h"
#include "systemconfiguration.h"
#include "udpserver.h"
#include <qtestcase.h>
#include <QElapsedTimer>
#include <qtestsupport_core.h>

tst_udpserver::tst_udpserver() { }
tst_udpserver::~tst_udpserver() = default;

static bool waitForDatagram(
  QUdpSocket& socket,
  int timeout = 1000)
{
  QElapsedTimer timer;
  timer.start();

  while (timer.elapsed() < timeout)
  {
    if (socket.hasPendingDatagrams())
      return true;

    QCoreApplication::processEvents();
    QTest::qWait(1);
  }

  return false;
}

static void processEvents(int timeoutMs = 100)
{
  QElapsedTimer timer;
  timer.start();

  while (timer.elapsed() < timeoutMs)
  {
    QCoreApplication::processEvents(
      QEventLoop::AllEvents,
      10);

    QTest::qWait(1);
  }
}

void tst_udpserver::test_ping() {
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

  UdpServer server(
    manager,
    scheduler);

  QVERIFY(server.start(0));
  QVERIFY(server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  // Формируем Ping
  PacketWriter writer;
  writer.begin(PacketType::Ping);

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
      writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      server.port());

  QCOMPARE(
    bytes,
    qint64(writer.size()));


  // Ждём ответ
  //QTest::qWait(100);
  QTRY_VERIFY(client.waitForReadyRead(100));
  //QVERIFY(waitForDatagram(client));

  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем
  QByteArray data;
  data.resize(
    client.pendingDatagramSize());

  client.readDatagram(
    data.data(),
    data.size());

  // Проверяем
  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(
      data.constData()),
    data.size());

  QVERIFY(
    reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::Pong);

  server.stop();
}

void tst_udpserver::test_invalid_packet()
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

  UdpServer server(
    manager,
    scheduler);
  // запускаем сервер
  QVERIFY(server.start(0));
  QVERIFY(server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(client.bind(QHostAddress::LocalHost, 0));

  // Формируем пакет
  QByteArray garbage;
  garbage.append(char(0x11));
  garbage.append(char(0x22));
  garbage.append(char(0x33));
  garbage.append(char(0x44));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      garbage,
      QHostAddress::LocalHost,
      server.port());

  QCOMPARE(
    bytes,
    4);

  processEvents();

  QVERIFY(!client.waitForReadyRead(100));
  QTRY_VERIFY(!client.hasPendingDatagrams());
  QVERIFY(server.isRunning());

  server.stop();
}

void tst_udpserver::test_unknown_packet()
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

  UdpServer server(manager, scheduler);

  QVERIFY(server.start(0));
  QVERIFY(server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  // Формируем пакет с неподдерживаемым PacketType
  PacketWriter writer;
  writer.begin(static_cast<PacketType>(255));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
    reinterpret_cast<const char*>(
      writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      server.port());

  QCOMPARE(
    bytes,
    qint64(writer.size()));

  processEvents();

  QVERIFY(!client.waitForReadyRead(100));
  QTRY_VERIFY(!client.hasPendingDatagrams());
  QVERIFY(server.isRunning());

  server.stop();
}
