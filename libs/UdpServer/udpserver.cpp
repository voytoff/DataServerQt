#include "udpserver.h"
#include <qdebug.h>

namespace qds
{

UdpServer::UdpServer(
  SubscriptionManager& subscriptions,
  LiveScheduler& scheduler,
  QObject* parent)
  : QObject(parent)
  , m_subscriptions(subscriptions)
  , m_scheduler(scheduler)
{
}

bool UdpServer::start(uint16_t port)
{
  connect(&m_socket,
          &QUdpSocket::readyRead,
          this,
          &UdpServer::onReadyRead);

  bool ok = m_socket.bind(
    QHostAddress::AnyIPv4,
    port);

  qDebug() << "bind:" << ok
           << m_socket.localAddress()
           << m_socket.localPort();
  if (!ok)  qDebug() << m_socket.errorString();

  return ok;
}

void UdpServer::stop()
{
  m_socket.close();
}

uint16_t UdpServer::port() const noexcept {
  return m_socket.localPort();
}

bool UdpServer::isRunning() const noexcept {
  return m_socket.state() == QAbstractSocket::BoundState;
}

void UdpServer::onReadyRead()
{
  qDebug() << "readyRead";
  while (m_socket.hasPendingDatagrams())
  {
    QByteArray data;
    data.resize(
      m_socket.pendingDatagramSize());

    QHostAddress address;
    quint16 port = 0;

    m_socket.readDatagram(
      data.data(),
      data.size(),
      &address,
      &port);

    Endpoint endpoint;
    endpoint.address =
      address.toString().toStdString();
    endpoint.port = port;

    PacketReader reader;

    reader.append(
      reinterpret_cast<const std::byte*>(
        data.constData()),
      data.size());

    if (!reader.nextPacket())
      continue;

    processPacket(reader, endpoint);
  }
}

void UdpServer::processPacket(
  PacketReader& reader,
  const Endpoint& endpoint)
{
  switch (reader.packetType())
  {
  case PacketType::Ping:
    processPing(reader, endpoint);
    break;

  default:
    qDebug() << "Unknown packet type:" << static_cast<int>(reader.packetType());
    break;
  }
}

void UdpServer::processPing(
  PacketReader&,
  const Endpoint& endpoint)
{
  qDebug() << "processPing";

  PacketWriter writer;
  writer.begin(PacketType::Pong);

  const QHostAddress address(
    QString::fromStdString(
      endpoint.address));

  const auto sent = m_socket.writeDatagram(
    reinterpret_cast<const char*>(
      writer.data()),
    static_cast<qint64>(
      writer.size()),
    address,
    endpoint.port);

  qDebug() << sent;
}

}