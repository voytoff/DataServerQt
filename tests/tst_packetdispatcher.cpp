#include "tst_packetdispatcher.h"
#include "testsrv.h"
#include <QtCore/qtestsupport_core.h>
#include <QtTest/qtestcase.h>

#include "systemconfiguration.h"

tst_packetdispatcher::tst_packetdispatcher() { }
tst_packetdispatcher::~tst_packetdispatcher() = default;

void tst_packetdispatcher::tst_packetdispatcher_InvalidTag()
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
  constexpr TagId tags[] { {0}, {2} }; // 2 - не существует

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

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::InvalidTag);
  QVERIFY(response.id.value == 0);

  // проверим наличие подписок
  QCOMPARE(srv.manager.size(), 0u);

  QVERIFY(srv.scheduler.step());
  QCOMPARE(srv.publisherSender.sendCount, 0u);

  srv.server.stop();
  QVERIFY(!srv.server.isRunning());
}
