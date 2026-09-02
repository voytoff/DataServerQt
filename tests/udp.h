#pragma once

#include <QUdpSocket>
#include <qtestcase.h>
#include <qtestsupport_core.h>
#include "packetreader.h"

static void waitPacket(QUdpSocket &client, QByteArray &data, qds::PacketReader &reader, const qds::PacketType &type)
{
  do {
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
  } while (reader.packetType() != type);
}