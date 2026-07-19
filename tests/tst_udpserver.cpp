#include "tst_udpserver.h"
#include "isender.h"
#include "livescheduler.h"
#include "publisher.h"
#include "subscriptionmanager.h"
#include "systemconfiguration.h"
#include "testsrv.h"
#include "udpserver.h"
#include <qtestcase.h>
#include <QElapsedTimer>
#include <qtestsupport_core.h>

tst_udpserver::tst_udpserver() { }
tst_udpserver::~tst_udpserver() = default;

static qds::SystemConfiguration createTestConfig(const qds::TagId* tags, int tagCount)
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg;

  ModuleInfo m0;
  m0.id = {0};
  cfg.addModule(m0);

  for (int i = 0; i < tagCount; i++) {
    TagId t = tags[i];
    TagInfo ti;
    ti.tag = t;
    ti.module = {0};
    cfg.addTag(ti);
  }

  return cfg;
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
  SystemConfiguration cfg;
  TestSrv srv(cfg);

  QVERIFY(srv.server.start(0));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем Ping
  PacketWriter writer;
  writer.begin(PacketType::Ping);

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));


  // Ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
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
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(reader.packetType(), PacketType::Pong);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_udpserver::test_ping_extraData()
{
  SystemConfiguration cfg;
  TestSrv srv(cfg);

  QVERIFY(srv.server.start(0));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем Ping
  PacketWriter writer;
  writer.begin(PacketType::Ping);

  // Добавляем лишние данные после Ping
  // Ping не должен содержать payload
  QByteArray garbage;
  garbage.append(100, char(0x77));
  writer.writeArray(garbage.data(), garbage.size());

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::ErrorResponse);

  ErrorResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.eof());

  QCOMPARE(response.code, ErrorCode::ExtraData);
  QCOMPARE(response.info, uint32_t(garbage.size()));

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_udpserver::test_invalid_packet()
{
  using namespace qds;
  SystemConfiguration cfg;

  TestSrv srv(cfg);

  // запускаем сервер
  QVERIFY(srv.server.start(0));
  QVERIFY(srv.server.isRunning());

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
      srv.server.port());

  QCOMPARE(bytes, 4);

  processEvents();

  QVERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::ErrorResponse);

  ErrorResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.eof());

  QCOMPARE(response.code, ErrorCode::InvalidPacket);
  QCOMPARE(response.info, 0);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_udpserver::test_packet_payloadSizeMismatch()
{
  using namespace qds;
  SystemConfiguration cfg;
  TestSrv srv(cfg);

  // запускаем сервер
  QVERIFY(srv.server.start(0));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(client.bind(QHostAddress::LocalHost, 0));

  // Формируем пакет
  PacketHeader hdr{};
  hdr.magic = ProtocolMagic;
  hdr.version = ProtocolVersion;
  hdr.type = PacketType::Ping;
  hdr.payloadSize = 100;

  QByteArray data;
  data.append(reinterpret_cast<const char*>(&hdr), sizeof(hdr));

  char payload[5] = {};
  data.append(payload, sizeof(payload));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      data,
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, data.size());

  processEvents();

  QVERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::ErrorResponse);

  ErrorResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.eof());

  QCOMPARE(response.code, ErrorCode::InvalidPacket);
  QCOMPARE(response.info, 0);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_udpserver::test_packet_payloadSizeTooSmall()
{
  using namespace qds;
  SystemConfiguration cfg;
  TestSrv srv(cfg);

  // запускаем сервер
  QVERIFY(srv.server.start(0));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(client.bind(QHostAddress::LocalHost, 0));

  // Формируем пакет
  PacketHeader hdr{};
  hdr.magic = ProtocolMagic;
  hdr.version = ProtocolVersion;
  hdr.type = PacketType::Ping;
  hdr.payloadSize = 24;

  QByteArray data;
  data.append(reinterpret_cast<const char*>(&hdr), sizeof(hdr));

  char payload[100] = {};
  data.append(payload, sizeof(payload));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      data,
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, data.size());

  processEvents();

  QVERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::ErrorResponse);

  ErrorResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.eof());

  QCOMPARE(response.code, ErrorCode::ExtraData);
  QVERIFY(response.info > 0);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
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
  // ждем ответ
  QVERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::ErrorResponse);

  ErrorResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.eof());

  QCOMPARE(response.code, ErrorCode::UnsupportedPacket);
  QCOMPARE(response.info, 0);

  server.stop();
  QVERIFY(!server.isRunning());
}

void tst_udpserver::test_subscribeList_ok()
{
  using namespace qds;
  // создаем конфигурацию
  constexpr TagId tags1[] { {0}, {1} };
  SystemConfiguration cfg = createTestConfig(tags1, std::size(tags1));

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(0));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

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
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));


  // Ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::SubscribeResponse);

  SubscribeResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.eof());

  QCOMPARE(response.result, SubscribeResult::Ok);
  QVERIFY(response.id.value > 0);

  const Subscription* sub = srv.manager.find(response.id);
  QVERIFY(sub != nullptr);

  QCOMPARE(sub->rate, PublishRate::Hz10);

  QCOMPARE(sub->tags.size(), size_t(2));
  QCOMPARE(sub->tags[0], TagId{0});
  QCOMPARE(sub->tags[1], TagId{1});

  srv.scheduler.tick();
  QCOMPARE(srv.sender.sendCount, 1u);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_udpserver::test_subscribeList_truncatedTagArray()
{
  using namespace qds;
  // создаем конфигурацию
  constexpr TagId tags1[] { {0}, {1} };
  SystemConfiguration cfg = createTestConfig(tags1, std::size(tags1));

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(0));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr TagId tags[]
    {
      {0},
      {1}
    };

  // Заявляем 3 тега, передаем только 2
  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.tagCount = std::size(tags) + 1;

  writer.write(req);
  writer.writeArray(tags, std::size(tags));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));


  // Ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::ErrorResponse);

  ErrorResponse err;
  QVERIFY(reader.read(err));

  QVERIFY(reader.eof());

  QCOMPARE(err.code, ErrorCode::InvalidRequest);
  QVERIFY(err.info == 0);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_udpserver::test_subscribeList_empty()
{
  using namespace qds;

  SystemConfiguration cfg;
  TestSrv srv(cfg);

  QVERIFY(srv.server.start(0));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.tagCount = 0;

  writer.write(req);

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::SubscribeResponse);

  SubscribeResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.eof());

  QCOMPARE(response.result, SubscribeResult::EmptyList);
  QCOMPARE(response.id, SubscriptionId{});;

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_udpserver::test_subscribeList_tooManyTags()
{
  using namespace qds;

  SystemConfiguration cfg;
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
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.tagCount = MaxSubscriptionTags + 1;

  writer.write(req);

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      server.port());

  QCOMPARE(bytes, qint64(writer.size()));


  // Ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::SubscribeResponse);

  SubscribeResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.eof());

  QCOMPARE(response.result, SubscribeResult::TooManyTags);
  QCOMPARE(response.id, SubscriptionId{});;

  server.stop();
  QVERIFY(!server.isRunning());
}

void tst_udpserver::test_subscribeList_invalidTag()
{
  using namespace qds;
  // создаем конфигурацию
  constexpr TagId tags1[] { {0}, {1} };
  SystemConfiguration cfg = createTestConfig(tags1, std::size(tags1));

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(0));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr TagId tags[]
  {
    {0},
    {1},
    {2}
  };
  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.tagCount = std::size(tags);

  writer.write(req);
  writer.writeArray(tags, std::size(tags));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::SubscribeResponse);

  SubscribeResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.eof());

  QCOMPARE(response.result, SubscribeResult::InvalidTag);
  QCOMPARE(response.id, SubscriptionId{});;

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_udpserver::test_subscribeList_invalidRate()
{
  using namespace qds;
  // создаем конфигурацию
  constexpr TagId tags1[] { {0}, {1} };
  SystemConfiguration cfg = createTestConfig(tags1, std::size(tags1));

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(0));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

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
  req.rate = static_cast<PublishRate>(0xFF);
  req.tagCount = std::size(tags);

  writer.write(req);
  writer.writeArray(tags, std::size(tags));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::SubscribeResponse);

  SubscribeResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.eof());

  QCOMPARE(response.result, SubscribeResult::InvalidRate);
  QCOMPARE(response.id, SubscriptionId{});;

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_udpserver::test_subscribeList_duplicateTag()
{
  using namespace qds;
  // создаем конфигурацию
  constexpr TagId tags1[] { {0}, {1} };
  SystemConfiguration cfg = createTestConfig(tags1, std::size(tags1));

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
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr TagId tags[]
    {
      {0},
      {1},
      {0}
    };
  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.tagCount = std::size(tags);

  writer.write(req);
  writer.writeArray(tags, std::size(tags));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      server.port());

  QCOMPARE(bytes, qint64(writer.size()));


  // Ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::SubscribeResponse);

  SubscribeResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.eof());

  QCOMPARE(response.result, SubscribeResult::DuplicateTag);
  QCOMPARE(response.id, SubscriptionId{});;

  server.stop();
  QVERIFY(!server.isRunning());
}

void tst_udpserver::test_subscribeList_emptyPayload()
{
  SystemConfiguration cfg;
  TestSrv srv(cfg);

  QVERIFY(srv.server.start(0));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Тело запроса SubscribeListRequest на подписку не отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::ErrorResponse);

  ErrorResponse err;
  QVERIFY(reader.read(err));

  QVERIFY(reader.eof());

  QCOMPARE(err.code, ErrorCode::InvalidRequest);
  QVERIFY(err.info == 0);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_udpserver::test_unsubscribe_ok()
{
  using namespace qds;
  // создаем конфигурацию
  constexpr TagId tags1[] { {0}, {1} };
  SystemConfiguration cfg = createTestConfig(tags1, std::size(tags1));

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

  QVERIFY(client.bind(QHostAddress::LocalHost, 0));

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
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      server.port());

  QCOMPARE(bytes, qint64(writer.size()));


  // Ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
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

  // подписка создана, теперь попробуем ее удалить ===============

  UnsubscribeRequest req2;
  req2.id = response.id;

  //writer.clear();
  writer.begin(PacketType::UnsubscribeRequest);
  writer.write(req2);

  // Отправляем
  const auto bytes2 =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      server.port());

  QCOMPARE(bytes2, qint64(writer.size()));

  // И опять ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ на удаление подписки
  data.resize(client.pendingDatagramSize());
  client.readDatagram(data.data(), data.size());

  reader.clear();
  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::UnsubscribeResponse);

  UnsubscribeResponse response2;
  QVERIFY(reader.read(response2));

  QVERIFY(reader.eof());

  QCOMPARE(response2.result, UnsubscribeResult::Ok);

  const Subscription* sub2 = manager.find(response.id);
  QVERIFY(sub2 == nullptr);

  scheduler.tick();
  QCOMPARE(sender.sendCount, 1u);


  server.stop();
  QVERIFY(!server.isRunning());
}

void tst_udpserver::test_unsubscribe_invalidId()
{
  using namespace qds;
  // создаем конфигурацию
  constexpr TagId tags1[] { {0}, {1} };
  SystemConfiguration cfg = createTestConfig(tags1, std::size(tags1));

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

  QVERIFY(client.bind(QHostAddress::LocalHost, 0));

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
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      server.port());

  QCOMPARE(bytes, qint64(writer.size()));


  // Ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
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

  // подписка создана, теперь попробуем удалить, но не какую то другую ===============

  UnsubscribeRequest req2;
  req2.id = { response.id.value + 1 };

  //writer.clear();
  writer.begin(PacketType::UnsubscribeRequest);
  writer.write(req2);

  // Отправляем
  const auto bytes2 =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      server.port());

  QCOMPARE(bytes2, qint64(writer.size()));

  // И опять ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ на удаление подписки
  data.resize(client.pendingDatagramSize());
  client.readDatagram(data.data(), data.size());

  reader.clear();
  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::UnsubscribeResponse);

  UnsubscribeResponse response2;
  QVERIFY(reader.read(response2));

  QVERIFY(reader.eof());

  QCOMPARE(response2.result, UnsubscribeResult::InvalidId);

  scheduler.tick();
  QCOMPARE(sender.sendCount, 1u);


  server.stop();
  QVERIFY(!server.isRunning());
}

void tst_udpserver::test_unsubscribe_extraData()
{
  using namespace qds;
  // создаем конфигурацию
  constexpr TagId tags1[] { {0}, {1} };
  SystemConfiguration cfg = createTestConfig(tags1, std::size(tags1));

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

  QVERIFY(client.bind(QHostAddress::LocalHost, 0));

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
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      server.port());

  QCOMPARE(bytes, qint64(writer.size()));


  // Ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
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

  // подписка создана, теперь попробуем ее удалить ===============

  UnsubscribeRequest req2;
  req2.id = response.id;

  //writer.clear();
  writer.begin(PacketType::UnsubscribeRequest);
  writer.write(req2);

  // но допишем в запрос мусор. eof не сработает
  uint32_t garbage = 123;
  writer.write(garbage);

  // Отправляем
  const auto bytes2 =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      server.port());

  QCOMPARE(bytes2, qint64(writer.size()));

  processEvents();

  QVERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  reader.clear();

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::ErrorResponse);

  ErrorResponse err;
  QVERIFY(reader.read(err));

  QVERIFY(reader.eof());

  QCOMPARE(err.code, ErrorCode::ExtraData);
  QVERIFY(err.info == 4);


  server.stop();
  QVERIFY(!server.isRunning());
}

void tst_udpserver::test_unsubscribe_emptyPayload()
{
  SystemConfiguration cfg;
  TestSrv srv(cfg);

  QVERIFY(srv.server.start(0));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::UnsubscribeRequest);

  // Не отправляем тело UnsubscribeRequest
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем ответ
  QByteArray data;
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::ErrorResponse);

  ErrorResponse err;
  QVERIFY(reader.read(err));

  QVERIFY(reader.eof());

  QCOMPARE(err.code, ErrorCode::InvalidRequest);
  QVERIFY(err.info == 0);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_udpserver::test_ping_followedByPing()
{
  using namespace qds;
  SystemConfiguration cfg;
  TestSrv srv(cfg);

  QVERIFY(srv.server.start(0));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  PacketWriter w1;
  w1.begin(PacketType::Ping);

  PacketWriter w2;
  w2.begin(PacketType::Ping);

  QByteArray data;
  data.append(reinterpret_cast<const char*>(w1.data()), w1.size());
  data.append(reinterpret_cast<const char*>(w2.data()), w2.size());

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      data,
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(data.size()));

  // Ждём ответ
  QTRY_VERIFY(client.waitForReadyRead(100));
  QTRY_VERIFY(client.hasPendingDatagrams());

  // Читаем
  data.resize(
    client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  // Проверяем
  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::ErrorResponse);

  ErrorResponse err;
  QVERIFY(reader.read(err));

  QVERIFY(reader.eof());

  QCOMPARE(err.code, ErrorCode::ExtraData);
  QVERIFY(err.info == 0);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}
