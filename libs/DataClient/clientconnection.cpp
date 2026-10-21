#include "clientconnection.h"

#include "packetreader.h"
#include "packetwriter.h"
#include "protocol/publishheader.h"

namespace qds
{

ClientConnection::ClientConnection(
  QObject* parent)
  : QObject(parent)
{
  connect(
    &m_socket,
    &QUdpSocket::readyRead,
    this,
    &ClientConnection::onReadyRead);
}

bool ClientConnection::open(
  const QHostAddress& address,
  quint16 port,
  quint16 localPort)
{
  close();

  if (!m_socket.bind(
        QHostAddress::AnyIPv4,
        localPort))
  {
    emit errorOccurred(
      m_socket.errorString());

    return false;
  }

  m_serverAddress = address;
  m_serverPort = port;

  return true;
}

bool ClientConnection::subscribe(
  std::span<const SignalId> signalIds,
  PublishRate rate)
{
  if (signalIds.empty() ||
      signalIds.size() > MaxSubscriptionSignals ||
      m_serverPort == 0 ||
      m_socket.state() !=
        QAbstractSocket::BoundState)
  {
    return false;
  }

  PacketWriter writer;

  writer.begin(
    PacketType::SubscribeListRequest);

  writer.write(
    SubscribeListRequest{
      .rate = rate,
      .signalCount =
      static_cast<uint32_t>(
        signalIds.size())
    });

  writer.writeArray(
    signalIds.data(),
    signalIds.size());

  const auto written =
    m_socket.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      static_cast<qint64>(
        writer.size()),
      m_serverAddress,
      m_serverPort);

  if (written !=
      static_cast<qint64>(writer.size()))
  {
    emit errorOccurred(
      m_socket.errorString());

    return false;
  }

  m_pendingSignals.assign(
    signalIds.begin(),
    signalIds.end());

  return true;
}

void ClientConnection::close()
{
  m_socket.close();

  m_serverPort = 0;
  m_pendingSignals.clear();
}

void ClientConnection::onReadyRead()
{
  while (m_socket.hasPendingDatagrams())
  {
    QByteArray datagram;

    datagram.resize(
      static_cast<qsizetype>(
        m_socket.pendingDatagramSize()));

    QHostAddress senderAddress;
    quint16 senderPort = 0;

    const qint64 received =
      m_socket.readDatagram(
        datagram.data(),
        datagram.size(),
        &senderAddress,
        &senderPort);

    if (received < 0)
      continue;

    if (senderAddress != m_serverAddress ||
        senderPort != m_serverPort)
      continue;

    PacketReader reader;

    reader.append(
      reinterpret_cast<const std::byte*>(
        datagram.constData()),
      static_cast<std::size_t>(
        received));

    if (!reader.nextPacket())
      continue;

    switch (reader.packetType())
    {
    case PacketType::SubscribeResponse:
    {
      SubscribeResponse response{};

      if (!reader.read(response) ||
          reader.remaining() != 0)
        break;

      if (response.result == SubscribeResult::Ok)
      {
        emit subscribed(response.id);
      }
      else
      {
        emit errorOccurred(
          tr("Сервер отклонил подписку"));
      }

      break;
    }

    case PacketType::LiveData:
    {
      PublishHeader header{};

      if (!reader.read(header))
        break;

      if (header.valueCount >
          MaxSubscriptionSignals)
        break;

      std::vector<double> values(
        header.valueCount);

      if (!reader.readArray(
            values.data(),
            values.size()))
        break;

      if (reader.remaining() != 0)
        break;

      emit dataReceived(
        header.subscriptionId,
        header.timestamp,
        std::move(values));

      break;
    }

    default:
      break;
    }
  }
}

}