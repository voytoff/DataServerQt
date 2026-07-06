#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QDataStream>
#include <QTimer>
#include <QElapsedTimer>

#include "PacketType.h"

using namespace qds;

class Client : public QObject
{
  Q_OBJECT

public:
  Client()
  {
    connect(&socket, &QTcpSocket::connected,
            this, &Client::onConnected);

    connect(&socket, &QTcpSocket::readyRead,
            this, &Client::onReadyRead);

    socket.connectToHost("127.0.0.1", 12345);

    hzTimer.start();
    statTimer.start();
  }

private slots:

  void onConnected()
  {
    qDebug() << "Connected";

    sendSubscribeRange(0, 500);
  }

  void onReadyRead()
  {
  qDebug() << "Bytes received:" << socket.bytesAvailable();
    buffer.append(socket.readAll());

    parse();
  }

private:

  void sendSubscribeRange(uint32_t from, uint32_t to)
  {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);

    out << from << to;

    sendPacket(PacketType::SubscribeRange, payload);
  }

  void sendPacket(PacketType type, const QByteArray& payload)
  {
    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);

    PacketHeader header;
    header.type = (uint16_t)type;
    header.size = payload.size();

    out.writeRawData(reinterpret_cast<char*>(&header), sizeof(header));
    out.writeRawData(payload.data(), payload.size());

    socket.write(packet);
  }

  void parse()
  {
    while (buffer.size() > (int)sizeof(uint32_t))
    {
      QDataStream in(buffer);

      quint32 count;
      in >> count;

      int needed = sizeof(quint32) + count * (sizeof(uint32_t) + sizeof(double));

      if (buffer.size() < needed)
        return;

      buffer.remove(0, sizeof(quint32));

      for (quint32 i = 0; i < count; ++i)
      {
        uint32_t id;
        double value;

        in >> id >> value;

        if (i < 5)
          qDebug() << "Tag" << id << ":" << value;
      }

      frames++;

      if (statTimer.elapsed() >= 1000)
      {
        qDebug() << "HZ:" << frames;
        frames = 0;
        statTimer.restart();
      }

      buffer.remove(0, count * (sizeof(uint32_t) + sizeof(double)));
    }
  }

private:
  QTcpSocket socket;
  QByteArray buffer;

  QElapsedTimer hzTimer;
  QElapsedTimer statTimer;

  int frames = 0;
};

#endif // CLIENT_H
