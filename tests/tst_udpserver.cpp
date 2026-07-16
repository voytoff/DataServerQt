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
    cfg,
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
  QVERIFY(!server.isRunning());
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
    cfg,
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

  UdpServer server(cfg, manager, scheduler);

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

void tst_udpserver::test_subscribeList_ok()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg;

  ModuleInfo m0;
  m0.id = {0};
  cfg.addModule(m0);

  TagInfo t0;
  t0.tag = {0};
  t0.module = {0};
  cfg.addTag(t0);

  TagInfo t1;
  t1.tag = {1};
  t1.module = {0};
  cfg.addTag(t1);

  SubscriptionManager manager;
  Publisher publisher;
  LiveStorage storage(cfg);
  TestSender sender;

  LiveScheduler scheduler(
    storage,
    manager,
    publisher,
    sender);

  UdpServer server(cfg, manager, scheduler);

  QVERIFY(server.start(0));
  QVERIFY(server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr TagId tags[]
    {
      {0},
      {1}
    };
  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.tagCount = std::size(tags);

  writer.write(req);
  writer.writeArray(tags, std::size(tags));

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
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  // Проверяем
  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::SubscribeResponse);

  SubscribeResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.eof());

  QCOMPARE(response.result, SubscribeResult::Ok);
  QVERIFY(response.id.value > 0);

  const Subscription* sub = manager.find(response.id);
  QVERIFY(sub != nullptr);

  QCOMPARE(sub->rate, PublishRate::Hz10);

  QCOMPARE(sub->tags.size(), size_t(2));
  QCOMPARE(sub->tags[0], TagId{0});
  QCOMPARE(sub->tags[1], TagId{1});

  scheduler.tick();
  QCOMPARE(sender.sendCount, 1u);

  server.stop();
  QVERIFY(!server.isRunning());
}
