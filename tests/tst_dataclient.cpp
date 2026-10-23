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

  const RequestId requestId = request.requestId;

  QVERIFY(requestId.value != 0);

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

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(request.constData()),
    static_cast<std::size_t>(request.size()));

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeListRequest);

  SubscribeListRequest subscribeRequest{};

  QVERIFY(reader.read(subscribeRequest));

  const RequestId requestId = subscribeRequest.requestId;

  QVERIFY(requestId.value != 0);

  // Имитируем успешный ответ сервера.
  PacketWriter writer;
  const SubscriptionId subscriptionId{42};

  writer.begin(
    PacketType::SubscribeResponse);

  writer.write(
    SubscribeResponse{
      .requestId = requestId,
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

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(request.constData()),
    static_cast<std::size_t>(request.size()));

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeListRequest);

  SubscribeListRequest subscribeRequest{};

  QVERIFY(reader.read(subscribeRequest));

  const RequestId requestId = subscribeRequest.requestId;

  QVERIFY(requestId.value != 0);

  // Подтверждаем подписку.
  PacketWriter writer;
  const SubscriptionId subscriptionId{42};

  writer.begin(PacketType::SubscribeResponse);
  writer.write(SubscribeResponse{
    .requestId = requestId,
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

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(request.constData()),
    static_cast<std::size_t>(request.size()));

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeListRequest);

  SubscribeListRequest subscribeRequest{};

  QVERIFY(reader.read(subscribeRequest));

  const RequestId requestId = subscribeRequest.requestId;

  QVERIFY(requestId.value != 0);


  // Подтверждаем подписку.
  PacketWriter writer;
  const SubscriptionId subscriptionId{7};

  writer.begin(PacketType::SubscribeResponse);
  writer.write(SubscribeResponse{
    .requestId = requestId,
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

  reader.clear();

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

  const RequestId unsubscribeRequestId =
    unsubscribeRequest.requestId;

  QVERIFY(unsubscribeRequestId.value != 0);

  QCOMPARE(reader.remaining(), std::size_t{0});
  QCOMPARE(reader.trailingBytes(), std::size_t{0});

  // Подтверждаем отмену подписки.
  writer.clear();

  writer.begin(PacketType::UnsubscribeResponse);
  writer.write(UnsubscribeResponse{
    .requestId = unsubscribeRequestId,
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

void tst_dataclient::protocolError()
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

  QCOMPARE(client.subscriptionId(), SubscriptionId{});
  QVERIFY(client.signalIds().empty());

  // После отказа клиент должен разрешать новую подписку.
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

  const std::array<SignalId, 3> signalIds = {
    SignalId{1},
    SignalId{2},
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

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(request.constData()),
    static_cast<std::size_t>(request.size()));

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeListRequest);

  SubscribeListRequest subscribeRequest{};

  QVERIFY(reader.read(subscribeRequest));

  const RequestId requestId = subscribeRequest.requestId;

  QVERIFY(requestId.value != 0);

  // Возвращаем ошибку.
  PacketWriter writer;

  writer.begin(PacketType::SubscribeResponse);

  writer.write(SubscribeResponse{
    .requestId = requestId,
    .result = SubscribeResult::InvalidSignal,
    .id = {}
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
    QString("Сервер отклонил подписку, код: %1").arg(
      static_cast<int>(SubscribeResult::InvalidSignal)));

  QCOMPARE(client.subscriptionId(), SubscriptionId{});
  QVERIFY(client.signalIds().empty());

  // После отказа клиент должен разрешать новую подписку.
  QVERIFY(client.subscribe(
    signalIds,
    PublishRate::Hz10));
}

void tst_dataclient::subscribeTimeout()
{
  using namespace qds;

  QUdpSocket server;

  QVERIFY(server.bind(
    QHostAddress::LocalHost, 0));

  ClientConnection client;

  QVERIFY(client.open(
    QHostAddress::LocalHost,
    server.localPort()));

  const std::array<SignalId, 2> signalIds = {
    SignalId{1},
    SignalId{2}
  };

  QSignalSpy errorSpy(
    &client,
    &ClientConnection::errorOccurred);

  QVERIFY(errorSpy.isValid());

  QVERIFY(client.subscribe(
    signalIds,
    PublishRate::Hz10));

  // Сервер намеренно не отвечает.
  QTRY_VERIFY_WITH_TIMEOUT(
    server.hasPendingDatagrams(), 1000);

  QTRY_COMPARE_WITH_TIMEOUT(
    errorSpy.count(), 1, 6000);

  QCOMPARE(
    errorSpy.takeFirst().at(0).toString(),
    QString("Превышено время ожидания подтверждения подписки"));

  QCOMPARE(
    client.subscriptionId(),
    SubscriptionId{});

  QVERIFY(client.signalIds().empty());

  // После тайм-аута можно повторить запрос.
  QVERIFY(client.subscribe(
    signalIds,
    PublishRate::Hz10));
}

void tst_dataclient::unsubscribeTimeout()
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
    SignalId{2}
  };

  QSignalSpy errorSpy(
    &client,
    &ClientConnection::errorOccurred);

  QVERIFY(errorSpy.isValid());

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

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(request.constData()),
    static_cast<std::size_t>(request.size()));

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeListRequest);

  SubscribeListRequest subscribeRequest{};

  QVERIFY(reader.read(subscribeRequest));

  const RequestId requestId = subscribeRequest.requestId;

  QVERIFY(requestId.value != 0);

  // Подтверждаем подписку.

  PacketWriter writer;
  const SubscriptionId subscriptionId{7};

  writer.begin(PacketType::SubscribeResponse);
  writer.write(SubscribeResponse{
    .requestId = requestId,
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

  // Отправляем запрос отписки.
  QVERIFY(client.unsubscribe());

  QTRY_VERIFY_WITH_TIMEOUT(
    server.hasPendingDatagrams(), 1000);

  request.resize(server.pendingDatagramSize());

  QVERIFY(server.readDatagram(
            request.data(),
            request.size(),
            &clientAddress,
            &clientPort) > 0);

  // Сервер намеренно не отвечает на отписку.
  QTRY_COMPARE_WITH_TIMEOUT(
    errorSpy.count(), 1, 6000);

  QCOMPARE(
    errorSpy.takeFirst().at(0).toString(),
    QString("Превышено время ожидания подтверждения отписки"));

  // Подписка локально сохраняется.
  QCOMPARE(
    client.subscriptionId(),
    subscriptionId);

  QCOMPARE(
    client.signalIds(),
    std::vector<SignalId>(
      signalIds.begin(),
      signalIds.end()));

  // После тайм-аута разрешается повторная отписка.
  QVERIFY(client.unsubscribe());
}

void tst_dataclient::subscribeRetry()
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
    SignalId{2}
  };

  QSignalSpy subscribedSpy(
    &client,
    &ClientConnection::subscribed);

  QSignalSpy errorSpy(
    &client,
    &ClientConnection::errorOccurred);

  QVERIFY(subscribedSpy.isValid());
  QVERIFY(errorSpy.isValid());

  QVERIFY(client.subscribe(
    signalIds,
    PublishRate::Hz100));

  // Получаем первый запрос
  QTRY_VERIFY_WITH_TIMEOUT(
    server.hasPendingDatagrams(), 1000);

  QByteArray request1;
  request1.resize(server.pendingDatagramSize());

  QHostAddress clientAddress;
  quint16 clientPort = 0;

  QCOMPARE(
    server.readDatagram(
      request1.data(),
      request1.size(),
      &clientAddress,
      &clientPort),
    qint64(request1.size()));

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(request1.constData()),
    request1.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeListRequest);

  SubscribeListRequest req1{};
  QVERIFY(reader.read(req1));

  QVERIFY(req1.requestId.value != 0);

  // Первый ответ намеренно не отправляем.

  // Ждём автоматическую повторную отправку
  QTRY_VERIFY_WITH_TIMEOUT(
    server.hasPendingDatagrams(), 2000);

  QByteArray request2;
  request2.resize(server.pendingDatagramSize());

  QCOMPARE(
    server.readDatagram(
      request2.data(),
      request2.size(),
      nullptr,
      nullptr),
    qint64(request2.size()));

  // Повторный пакет должен полностью совпадать
  QCOMPARE(request2, request1);

  reader.clear();

  reader.append(
    reinterpret_cast<const std::byte*>(request2.constData()),
    request2.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeListRequest);

  SubscribeListRequest req2{};
  QVERIFY(reader.read(req2));

  QCOMPARE(req2.requestId, req1.requestId);

  // Подтверждаем подписку после повторного запроса
  PacketWriter writer;

  const SubscriptionId subscriptionId{7};

  writer.begin(PacketType::SubscribeResponse);

  writer.write(SubscribeResponse{
    .requestId = req2.requestId,
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

  // Клиент должен успешно завершить подписку
  QTRY_COMPARE_WITH_TIMEOUT(
    subscribedSpy.count(), 1, 1000);

  QCOMPARE(client.subscriptionId(), subscriptionId);

  QCOMPARE(client.signalIds().size(), signalIds.size());

  QCOMPARE(client.signalIds()[0], signalIds[0]);
  QCOMPARE(client.signalIds()[1], signalIds[1]);

  QCOMPARE(errorSpy.count(), 0);
}

void tst_dataclient::unsubscribeRetry()
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

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(request.constData()),
    static_cast<std::size_t>(request.size()));

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeListRequest);

  SubscribeListRequest subscribeRequest{};

  QVERIFY(reader.read(subscribeRequest));

  const RequestId requestId = subscribeRequest.requestId;

  QVERIFY(requestId.value != 0);


  // Подтверждаем подписку.
  PacketWriter writer;
  const SubscriptionId subscriptionId{7};

  writer.begin(PacketType::SubscribeResponse);
  writer.write(SubscribeResponse{
    .requestId = requestId,
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

  reader.clear();

  reader.append(
    reinterpret_cast<const std::byte*>(request.constData()),
    static_cast<std::size_t>(request.size()));

  const QByteArray firstUnsubscribeRequest = request;

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::UnsubscribeRequest);

  UnsubscribeRequest unsubscribeRequest{};

  QVERIFY(reader.read(unsubscribeRequest));

  QCOMPARE(
    unsubscribeRequest.id,
    subscriptionId);

  const RequestId unsubscribeRequestId =
    unsubscribeRequest.requestId;

  QVERIFY(unsubscribeRequestId.value != 0);

  QCOMPARE(reader.remaining(), std::size_t{0});
  QCOMPARE(reader.trailingBytes(), std::size_t{0});


  // Ждём автоматическую повторную отправку
  QTRY_VERIFY_WITH_TIMEOUT(
    server.hasPendingDatagrams(), 2000);

  QByteArray repeatedRequest;
  repeatedRequest.resize(server.pendingDatagramSize());

  QCOMPARE(
    server.readDatagram(
      repeatedRequest.data(),
      repeatedRequest.size()),
    qint64(repeatedRequest.size()));

  // Проверяем полное совпадение пакетов
  QCOMPARE(repeatedRequest, firstUnsubscribeRequest);

  reader.clear();

  reader.append(
    reinterpret_cast<const std::byte*>(
      repeatedRequest.constData()),
    static_cast<std::size_t>(repeatedRequest.size()));

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::UnsubscribeRequest);

  UnsubscribeRequest repeatedUnsubscribe{};

  QVERIFY(reader.read(repeatedUnsubscribe));

  QCOMPARE(
    repeatedUnsubscribe.requestId,
    unsubscribeRequestId);

  QCOMPARE(
    repeatedUnsubscribe.id,
    subscriptionId);

  QCOMPARE(reader.remaining(), std::size_t{0});
  QCOMPARE(reader.trailingBytes(), std::size_t{0});


  // Подтверждаем отмену подписки.
  writer.clear();

  writer.begin(PacketType::UnsubscribeResponse);
  writer.write(UnsubscribeResponse{
    .requestId = unsubscribeRequestId,
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
