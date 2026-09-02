#include "tst_packetdispatcher.h"
#include "testsrv.h"
#include <QtCore/qtestsupport_core.h>
#include <QtTest/qtestcase.h>

#include "systemconfiguration.h"

tst_packetdispatcher::tst_packetdispatcher() { }
tst_packetdispatcher::~tst_packetdispatcher() = default;

void tst_packetdispatcher::tst_packetdispatcher_validSignal()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr SignalId signalIds[] { {17}, {23} };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount = std::size(signalIds);

  writer.write(req);
  writer.writeArray(signalIds, std::size(signalIds));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::Ok);
  QVERIFY(response.id == SubscriptionId{1});

  // проверим наличие подписок
  QCOMPARE(srv.manager.size(), 1u);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_invalidSignal()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr SignalId signalIds[] { {17}, {24} }; // 24 - не существует

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount = std::size(signalIds);

  writer.write(req);
  writer.writeArray(signalIds, std::size(signalIds));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::InvalidSignal);
  QVERIFY(response.id == SubscriptionId{0});

  // проверим наличие подписок
  QCOMPARE(srv.manager.size(), 0u);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_duplicateSignal()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr SignalId signalIds[] { {4}, {17}, {4} }; // 4 - 2 раза

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount = std::size(signalIds);

  writer.write(req);
  writer.writeArray(signalIds, std::size(signalIds));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::DuplicateSignal);
  QVERIFY(response.id == SubscriptionId{0});

  // проверим наличие подписок
  QCOMPARE(srv.manager.size(), 0u);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_emptyList()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount = 0;

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
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::EmptyList);
  QVERIFY(response.id == SubscriptionId{0});

  // проверим наличие подписок
  QCOMPARE(srv.manager.size(), 0u);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_tooManySignals()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount = MaxSubscriptionSignals + 1;

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
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::TooManySignals);
  QVERIFY(response.id == SubscriptionId{0});

  // проверим наличие подписок
  QCOMPARE(srv.manager.size(), 0u);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_invalidRate()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr SignalId signalIds[] { {17}, {23} };

  SubscribeListRequest req;
  req.rate = static_cast<PublishRate>(0xFF);
  req.signalCount = std::size(signalIds);

  writer.write(req);
  writer.writeArray(signalIds, std::size(signalIds));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::InvalidRate);
  QVERIFY(response.id == SubscriptionId{0});

  // проверим наличие подписок
  QCOMPARE(srv.manager.size(), 0u);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_unsubscribe()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr SignalId signalIds[] { {17}, {23} };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount = std::size(signalIds);

  writer.write(req);
  writer.writeArray(signalIds, std::size(signalIds));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::Ok);
  QVERIFY(response.id == SubscriptionId{1});

  // проверим наличие подписок
  QCOMPARE(srv.manager.size(), 1u);


  UnsubscribeRequest req2;
  req2.id = response.id;

  writer.begin(PacketType::UnsubscribeRequest);
  writer.write(req2);

  // Отправляем
  const auto bytes2 =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes2, qint64(writer.size()));

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  data.resize(client.pendingDatagramSize());
  client.readDatagram(data.data(), data.size());

  reader.clear();
  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(reader.packetType(), PacketType::UnsubscribeResponse);

  UnsubscribeResponse response2;
  QVERIFY(reader.read(response2));

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response2.result, UnsubscribeResult::Ok);

  // проверим отсутствие подписок
  QCOMPARE(srv.manager.size(), 0u);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_unsubscribeinvalidId()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr SignalId signalIds[] { {17}, {23} };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount = std::size(signalIds);

  writer.write(req);
  writer.writeArray(signalIds, std::size(signalIds));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::Ok);
  QVERIFY(response.id == SubscriptionId{1});

  // проверим наличие подписок
  QCOMPARE(srv.manager.size(), 1u);


  UnsubscribeRequest req2;
  req2.id = SubscriptionId{777};

  writer.begin(PacketType::UnsubscribeRequest);
  writer.write(req2);

  // Отправляем
  const auto bytes2 =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes2, qint64(writer.size()));

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  data.resize(client.pendingDatagramSize());
  client.readDatagram(data.data(), data.size());

  reader.clear();
  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(reader.packetType(), PacketType::UnsubscribeResponse);

  UnsubscribeResponse response2;
  QVERIFY(reader.read(response2));

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response2.result, UnsubscribeResult::InvalidId);

  // проверим отсутствие подписок
  QCOMPARE(srv.manager.size(), 1u);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_ping()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
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
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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
  QCOMPARE(reader.packetType(), PacketType::Pong);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_pingExtraData()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
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
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.code, ErrorCode::ExtraData);
  QCOMPARE(response.info, uint32_t(garbage.size()));

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_subscribeWithoutPayload()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
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
  req.signalCount = 2;

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
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.code, ErrorCode::InvalidRequest);
  QCOMPARE(
    response.info,
    uint32_t(0));

  // проверим отсутствие подписок
  QCOMPARE(srv.manager.size(), 0u);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_subscribeExtraData()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr SignalId signalIds[] { {17}, {23} };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount = std::size(signalIds);

  writer.write(req);
  writer.writeArray(signalIds, std::size(signalIds));

  // Добавляем лишние данные после SubscribeListRequest
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
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.code, ErrorCode::ExtraData);
  QCOMPARE(response.info, uint32_t(garbage.size()));

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_unknownPacket()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос
  PacketWriter writer;
  writer.begin(static_cast<PacketType>(1000));

  // Добавляем лишние данные
  QByteArray garbage;
  garbage.append(10, char(0xFF));
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
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QCOMPARE(response.code, ErrorCode::UnsupportedPacket);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_invalidPacket()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Добавляем лишние данные
  QByteArray garbage;
  garbage.append(32, char(0xFF));

  const auto bytes =
    client.writeDatagram(
      garbage.constData(),
      garbage.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(garbage.size()));

  // Ждём ответ
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QCOMPARE(response.code, ErrorCode::InvalidPacket);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_invalidSignalCount()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr SignalId signalIds[] { {17}, {4} };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount = 3;

  writer.write(req);
  writer.writeArray(signalIds, std::size(signalIds));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QCOMPARE(response.code, ErrorCode::InvalidRequest);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_subscriptionIdReuse()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr SignalId signalIds[] { {17}, {23} };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount = std::size(signalIds);

  writer.write(req);
  writer.writeArray(signalIds, std::size(signalIds));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::Ok);
  QVERIFY(response.id == SubscriptionId{1});

  // проверим наличие подписок
  QCOMPARE(srv.manager.size(), 1u);


  UnsubscribeRequest req2;
  req2.id = response.id;

  writer.begin(PacketType::UnsubscribeRequest);
  writer.write(req2);

  // Отправляем
  const auto bytes2 =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes2, qint64(writer.size()));

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  data.resize(client.pendingDatagramSize());
  client.readDatagram(data.data(), data.size());

  reader.clear();
  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(reader.packetType(), PacketType::UnsubscribeResponse);

  UnsubscribeResponse response2;
  QVERIFY(reader.read(response2));

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response2.result, UnsubscribeResult::Ok);

  // проверим отсутствие подписок
  QCOMPARE(srv.manager.size(), 0u);


/*====================================================/
 * Повторная подписка
*====================================================*/

  // Формируем запрос SubscribeListRequest
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr SignalId signalIds2[] { SignalId{4}, SignalId{23}, SignalId{17} };

  req.rate = PublishRate::Hz100;
  req.signalCount = std::size(signalIds2);

  writer.write(req);
  writer.writeArray(signalIds2, std::size(signalIds2));

  // Отправляем
  const auto bytes3 =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes3, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  // Читаем ответ
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  reader.clear();
  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::SubscribeResponse);

  QVERIFY(reader.read(response));

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::Ok);
  QVERIFY(response.id == SubscriptionId{2});

  // проверим наличие подписок
  QCOMPARE(srv.manager.size(), 1u);


  req2.id = response.id;

  writer.begin(PacketType::UnsubscribeRequest);
  writer.write(req2);

  // Отправляем
  const auto bytes4 =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes4, qint64(writer.size()));

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  data.resize(client.pendingDatagramSize());
  client.readDatagram(data.data(), data.size());

  reader.clear();
  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(reader.packetType(), PacketType::UnsubscribeResponse);

  QVERIFY(reader.read(response2));

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response2.result, UnsubscribeResult::Ok);

  // проверим отсутствие подписок
  QCOMPARE(srv.manager.size(), 0u);


/*====================================================/
 * Еще одна подписка
*====================================================*/

  // Формируем запрос SubscribeListRequest
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr SignalId signalIds3[] { SignalId{4} };

  req.rate = PublishRate::Hz1;
  req.signalCount = std::size(signalIds3);

  writer.write(req);
  writer.writeArray(signalIds3, std::size(signalIds3));

  // Отправляем
  const auto bytes5 =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes5, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  // Читаем ответ
  data.resize(client.pendingDatagramSize());

  client.readDatagram(data.data(), data.size());

  reader.clear();
  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  // Проверяем
  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::SubscribeResponse);

  QVERIFY(reader.read(response));

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::Ok);
  QVERIFY(response.id == SubscriptionId{3});

  // проверим наличие подписок
  QCOMPARE(srv.manager.size(), 1u);


  req2.id = response.id;

  writer.begin(PacketType::UnsubscribeRequest);
  writer.write(req2);

  // Отправляем
  const auto bytes6 =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes6, qint64(writer.size()));

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  data.resize(client.pendingDatagramSize());
  client.readDatagram(data.data(), data.size());

  reader.clear();
  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(reader.packetType(), PacketType::UnsubscribeResponse);

  QVERIFY(reader.read(response2));

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response2.result, UnsubscribeResult::Ok);

  // проверим отсутствие подписок
  QCOMPARE(srv.manager.size(), 0u);


  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

void tst_packetdispatcher::tst_packetdispatcher_unsubscribeExtraData()
{
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg = createTestConfig_calculate();

  TestSrv srv(cfg);

  QVERIFY(srv.server.start(srv.server.port()));
  QVERIFY(srv.server.isRunning());

  // Создаём клиент
  QUdpSocket client;

  QVERIFY(
    client.bind(QHostAddress::LocalHost, 0));

  // Формируем запрос SubscribeListRequest
  PacketWriter writer;
  writer.begin(PacketType::SubscribeListRequest);

  // Формируем запрос на подписку
  constexpr SignalId signalIds[] { {17}, {23} };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount = std::size(signalIds);

  writer.write(req);
  writer.writeArray(signalIds, std::size(signalIds));

  // Отправляем
  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes, qint64(writer.size()));

  // Ждём ответ
  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

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

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::Ok);
  QVERIFY(response.id == SubscriptionId{1});

  // проверим наличие подписок
  QCOMPARE(srv.manager.size(), 1u);


  UnsubscribeRequest req2;
  req2.id = response.id;

  writer.begin(PacketType::UnsubscribeRequest);
  writer.write(req2);

  QByteArray garbage;
  garbage.append(10, char(0x77));
  writer.writeArray(garbage.data(), garbage.size());

  // Отправляем
  const auto bytes2 =
    client.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      srv.server.port());

  QCOMPARE(bytes2, qint64(writer.size()));

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  data.resize(client.pendingDatagramSize());
  client.readDatagram(data.data(), data.size());

  reader.clear();

  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());
  QCOMPARE(reader.packetType(), PacketType::ErrorResponse);

  ErrorResponse response2;
  QVERIFY(reader.read(response2));

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response2.code, ErrorCode::ExtraData);
  QCOMPARE(response2.info, uint32_t(garbage.size()));

  // Подписка не должна быть удалена
  QCOMPARE(srv.manager.size(), 1u);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}

