#include "tst_dataclient.h"

#include "clientconnection.h"
#include "errorpackets.h"
#include "packetreader.h"
#include "protocol/subscriptionpackets.h"
#include "packetwriter.h"
#include "protocol/publishheader.h"

#include <QSignalSpy>

#include <QTest>
#include <QUdpSocket>
#include <QHostAddress>

#include <array>

tst_dataclient::tst_dataclient() = default;
tst_dataclient::~tst_dataclient() = default;

void tst_dataclient::subscribeRequest()
{
  using namespace qds;

  // Имитируем сервер.
  QUdpSocket server;

  QVERIFY(server.bind(
    QHostAddress::LocalHost,
    0));

  // Создаём клиент.
  ClientConnection client;

  QVERIFY(client.open(
    QHostAddress::LocalHost,
    server.localPort()));

  // Отправляем подписку.
  const std::array<SignalId, 3> signalIds = {
    SignalId{1},
    SignalId{2},
    SignalId{3}
  };

  QVERIFY(client.subscribe(
    signalIds,
    PublishRate::Hz10));

  // Ожидаем UDP-пакет.
  QTRY_VERIFY_WITH_TIMEOUT(
    server.hasPendingDatagrams(),
    1000);

  QByteArray datagram;

  datagram.resize(
    static_cast<qsizetype>(
      server.pendingDatagramSize()));

  const qint64 received =
    server.readDatagram(
      datagram.data(),
      datagram.size());

  QVERIFY(received > 0);

  // Разбираем пакет.
  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(
      datagram.constData()),
    static_cast<std::size_t>(received));

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeListRequest);

  SubscribeListRequest request{};

  QVERIFY(reader.read(request));

  QCOMPARE(
    request.rate,
    PublishRate::Hz10);

  QCOMPARE(
    request.signalCount,
    uint32_t{3});

  // Проверяем список сигналов.
  std::array<SignalId, 3> receivedSignals{};

  QVERIFY(reader.readArray(
    receivedSignals.data(),
    receivedSignals.size()));

  QCOMPARE(receivedSignals, signalIds);

  QCOMPARE(reader.remaining(), std::size_t{0});
  QCOMPARE(reader.trailingBytes(), std::size_t{0});
}

void tst_dataclient::subscribeResponse()
{
  using namespace qds;

  QUdpSocket server;

  QVERIFY(server.bind(
    QHostAddress::LocalHost,
    0));

  ClientConnection client;

  QVERIFY(client.open(
    QHostAddress::LocalHost,
    server.localPort()));

  QSignalSpy subscribedSpy(
    &client,
    &ClientConnection::subscribed);

  QVERIFY(subscribedSpy.isValid());

  const std::array<SignalId, 3> signalIds = {
    SignalId{1},
    SignalId{2},
    SignalId{3}
  };

  QVERIFY(client.subscribe(
    signalIds,
    PublishRate::Hz10));

  // Получаем запрос и определяем адрес клиента.
  QTRY_VERIFY_WITH_TIMEOUT(
    server.hasPendingDatagrams(),
    1000);

  QByteArray request;

  request.resize(
    static_cast<qsizetype>(
      server.pendingDatagramSize()));

  QHostAddress clientAddress;
  quint16 clientPort = 0;

  QVERIFY(server.readDatagram(
            request.data(),
            request.size(),
            &clientAddress,
            &clientPort) > 0);

  // Имитируем успешный ответ сервера.
  const SubscriptionId subscriptionId{42};

  PacketWriter writer;

  writer.begin(
    PacketType::SubscribeResponse);

  writer.write(
    SubscribeResponse{
      .result = SubscribeResult::Ok,
      .id = subscriptionId
    });

  QCOMPARE(
    server.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      static_cast<qint64>(
        writer.size()),
      clientAddress,
      clientPort),
    static_cast<qint64>(writer.size()));

  // Проверяем получение подтверждения.
  QTRY_COMPARE_WITH_TIMEOUT(
    subscribedSpy.count(),
    1,
    1000);

  QCOMPARE(
    client.subscriptionId(),
    subscriptionId);

  QCOMPARE(
    client.signalIds().size(),
    signalIds.size());

  for (std::size_t i = 0; i < signalIds.size(); ++i)
  {
    QCOMPARE(
      client.signalIds()[i],
      signalIds[i]);
  }
}

void tst_dataclient::liveData()
{
  using namespace qds;

  QUdpSocket server;

  QVERIFY(server.bind(QHostAddress::LocalHost, 0));

  ClientConnection client;

  QVERIFY(client.open(
    QHostAddress::LocalHost,
    server.localPort()));

  const std::array<SignalId, 3> signalIds = {
    SignalId{1},
    SignalId{2},
    SignalId{3}
  };

  QSignalSpy subscribedSpy(
    &client,
    &ClientConnection::subscribed);

  QSignalSpy dataSpy(
    &client,
    &ClientConnection::dataReceived);

  QVERIFY(subscribedSpy.isValid());
  QVERIFY(dataSpy.isValid());

  QVERIFY(client.subscribe(
    signalIds,
    PublishRate::Hz10));

  // Получаем запрос подписки.
  QTRY_VERIFY_WITH_TIMEOUT(
    server.hasPendingDatagrams(), 1000);

  QByteArray request;
  request.resize(server.pendingDatagramSize());

  QHostAddress clientAddress;
  quint16 clientPort = 0;

  QVERIFY(server.readDatagram(
            request.data(),
            request.size(),
            &clientAddress,
            &clientPort) > 0);

  const SubscriptionId subscriptionId{42};

  // Подтверждаем подписку.
  PacketWriter writer;

  writer.begin(PacketType::SubscribeResponse);
  writer.write(SubscribeResponse{
    .result = SubscribeResult::Ok,
    .id = subscriptionId
  });

  QCOMPARE(
    server.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      static_cast<qint64>(writer.size()),
      clientAddress,
      clientPort),
    static_cast<qint64>(writer.size()));

  QTRY_COMPARE_WITH_TIMEOUT(
    subscribedSpy.count(), 1, 1000);

  // Отправляем пакет с измерениями.
  const std::array<double, 3> values = {
    12.5, 25.0, -3.75
  };

  constexpr uint64_t timestamp = 123456789;

  writer.begin(PacketType::LiveData);

  writer.write(PublishHeader{
    .subscriptionId = subscriptionId,
    .sequence = 1,
    .timestamp = timestamp,
    .valueCount = static_cast<uint32_t>(values.size())
  });

  writer.writeArray(values.data(), values.size());

  QCOMPARE(
    server.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      static_cast<qint64>(writer.size()),
      clientAddress,
      clientPort),
    static_cast<qint64>(writer.size()));

  // Проверяем полученные данные.
  QTRY_COMPARE_WITH_TIMEOUT(
    dataSpy.count(), 1, 1000);

  const auto arguments = dataSpy.takeFirst();

  QCOMPARE(
    arguments.at(0).value<SubscriptionId>(),
    subscriptionId);

  QCOMPARE(
    arguments.at(1).value<uint64_t>(),
    timestamp);

  const auto receivedValues =
    arguments.at(2).value<std::vector<double>>();

  QCOMPARE(receivedValues.size(), values.size());

  for (std::size_t i = 0; i < values.size(); ++i)
    QCOMPARE(receivedValues[i], values[i]);
}

void tst_dataclient::unsubscribe()
{
  using namespace qds;

  QUdpSocket server;

  QVERIFY(server.bind(QHostAddress::LocalHost, 0));

  ClientConnection client;

  QVERIFY(client.open(
    QHostAddress::LocalHost,
    server.localPort()));

  const std::array<SignalId, 2> signalIds = {
    SignalId{1},
    SignalId{3}
  };

  QSignalSpy unsubscribedSpy(
    &client,
    &ClientConnection::unsubscribed);

  QSignalSpy dataSpy(
    &client,
    &ClientConnection::dataReceived);

  QVERIFY(unsubscribedSpy.isValid());
  QVERIFY(dataSpy.isValid());

  QVERIFY(client.subscribe(
    signalIds,
    PublishRate::Hz1));

  // Получаем запрос подписки.
  QTRY_VERIFY_WITH_TIMEOUT(
    server.hasPendingDatagrams(), 1000);

  QByteArray request;
  request.resize(server.pendingDatagramSize());

  QHostAddress clientAddress;
  quint16 clientPort = 0;

  QVERIFY(server.readDatagram(
            request.data(),
            request.size(),
            &clientAddress,
            &clientPort) > 0);

  const SubscriptionId subscriptionId{7};

  // Подтверждаем подписку.
  PacketWriter writer;

  writer.begin(PacketType::SubscribeResponse);
  writer.write(SubscribeResponse{
    .result = SubscribeResult::Ok,
    .id = subscriptionId
  });

  QCOMPARE(
    server.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      static_cast<qint64>(writer.size()),
      clientAddress,
      clientPort),
    static_cast<qint64>(writer.size()));

  QTRY_COMPARE_WITH_TIMEOUT(
    client.subscriptionId(),
    subscriptionId,
    1000);

  // Отправляем пакет с измерениями.
  const std::array<double, 2> values = {
    12.5, 25.0
  };

  constexpr uint64_t timestamp = 1000000;

  writer.begin(PacketType::LiveData);

  writer.write(PublishHeader{
    .subscriptionId = subscriptionId,
    .sequence = 1,
    .timestamp = timestamp,
    .valueCount = static_cast<uint32_t>(values.size())
  });

  writer.writeArray(values.data(), values.size());

  QCOMPARE(
    server.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      static_cast<qint64>(writer.size()),
      clientAddress,
      clientPort),
    static_cast<qint64>(writer.size()));

  // Проверяем полученные данные.
  QTRY_COMPARE_WITH_TIMEOUT(
    dataSpy.count(), 1, 1000);

  const auto arguments = dataSpy.takeFirst();

  QCOMPARE(
    arguments.at(0).value<SubscriptionId>(),
    subscriptionId);

  QVERIFY(client.unsubscribe());

  // Получаем запрос отмены подписки.
  QTRY_VERIFY_WITH_TIMEOUT(
    server.hasPendingDatagrams(), 1000);

  request.resize(server.pendingDatagramSize());

  QVERIFY(server.readDatagram(
            request.data(),
            request.size(),
            &clientAddress,
            &clientPort) > 0);

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(request.constData()),
    static_cast<std::size_t>(request.size()));

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::UnsubscribeRequest);

  UnsubscribeRequest unsubscribeRequest{};

  QVERIFY(reader.read(unsubscribeRequest));

  QCOMPARE(
    unsubscribeRequest.id,
    subscriptionId);

  QCOMPARE(reader.remaining(), std::size_t{0});
  QCOMPARE(reader.trailingBytes(), std::size_t{0});

  // Подтверждаем отмену подписки.
  writer.clear();

  writer.begin(PacketType::UnsubscribeResponse);
  writer.write(UnsubscribeResponse{
    .result = UnsubscribeResult::Ok,
  });

  QCOMPARE(
    server.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      static_cast<qint64>(writer.size()),
      clientAddress,
      clientPort),
    static_cast<qint64>(writer.size()));

  QTRY_COMPARE_WITH_TIMEOUT(
    unsubscribedSpy.count(), 1, 1000);

  QCOMPARE(unsubscribedSpy.takeFirst().count(), 0);

  QCOMPARE(client.subscriptionId(), SubscriptionId{});
  QVERIFY(client.signalIds().empty());

  QVERIFY(client.subscribe(
    signalIds,
    PublishRate::Hz10));
}

void tst_dataclient::subscribeRejected()
{
  using namespace qds;

  QUdpSocket server;

  QVERIFY(server.bind(QHostAddress::LocalHost, 0));

  ClientConnection client;

  QVERIFY(client.open(
    QHostAddress::LocalHost,
    server.localPort()));

  const std::array<SignalId, 2> signalIds = {
    SignalId{1},
    SignalId{3}
  };

  QSignalSpy errorSpy(
    &client,
    &ClientConnection::errorOccurred);

  QVERIFY(errorSpy.isValid());

  QVERIFY(client.subscribe(
    signalIds,
    PublishRate::Hz100));

  // Получаем запрос подписки.
  QTRY_VERIFY_WITH_TIMEOUT(
    server.hasPendingDatagrams(), 1000);

  QByteArray request;
  request.resize(server.pendingDatagramSize());

  QHostAddress clientAddress;
  quint16 clientPort = 0;

  QVERIFY(server.readDatagram(
            request.data(),
            request.size(),
            &clientAddress,
            &clientPort) > 0);

  // Возвращаем ошибку.
  PacketWriter writer;

  writer.begin(PacketType::ErrorResponse);
  writer.write(ErrorResponse{
    .code = ErrorCode::InvalidRequest,
  });

  QCOMPARE(
    server.writeDatagram(
      reinterpret_cast<const char*>(writer.data()),
      static_cast<qint64>(writer.size()),
      clientAddress,
      clientPort),
    static_cast<qint64>(writer.size()));

  // Проверяем полученные данные.
  QTRY_COMPARE_WITH_TIMEOUT(
    errorSpy.count(), 1, 1000);

  const auto arguments = errorSpy.takeFirst();

  QCOMPARE(
    arguments.at(0).value<QString>(),
    QString("Ошибка протокола: %1, info=0").arg(
      static_cast<int>(ErrorCode::InvalidRequest)));
}