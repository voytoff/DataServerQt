#include "udpserver.h"

namespace qds
{

UdpServer::UdpServer(SubscriptionManager &subscriptions, LiveScheduler &scheduler, QObject *parent)
  : QObject(parent)
  , m_subscriptions(subscriptions)
  , m_scheduler(scheduler) {

}

bool UdpServer::start(quint16 port)
{
  const bool ok =
    m_socket.bind(
      QHostAddress::AnyIPv4,
      port);

  if (!ok)
    return false;

  connect(&m_socket,
          &QUdpSocket::readyRead,
          this,
          &UdpServer::onReadyRead);

  return true;
}

void UdpServer::stop()
{
  m_socket.close();
}

void UdpServer::onReadyRead()
{
  while (m_socket.hasPendingDatagrams())
  {
    QByteArray datagram;
    datagram.resize(
      m_socket.pendingDatagramSize());

    QHostAddress address;
    quint16 port;

    m_socket.readDatagram(
      datagram.data(),
      datagram.size(),
      &address,
      &port);

    Endpoint ep;
    ep.address =
      address.toString().toStdString();
    ep.port = port;

    PacketReader reader;

    reader.append(
      reinterpret_cast<const std::byte*>(
        datagram.data()),
      datagram.size());

    if (!reader.nextPacket())
      continue;

    processPacket(reader, ep);
  }
}

void UdpServer::processPacket(
  PacketReader& reader,
  const Endpoint& endpoint)
{
  switch (reader.packetType())
  {
  case PacketType::Ping:
    //processPing(reader, endpoint);
    break;

  case PacketType::SubscribeList:
    //processSubscribeList(reader, endpoint);
    break;

  case PacketType::SubscribeRange:
    //processSubscribeRange(reader, endpoint);
    break;

  case PacketType::Unsubscribe:
    //processUnsubscribe(reader, endpoint);
    break;

  default:
    break;
  }
}

}