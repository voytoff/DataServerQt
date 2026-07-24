#include "udpserver.h"
#include <qdebug.h>

namespace qds
{

UdpServer::UdpServer(
  PacketDispatcher &dispatcher,
  QObject* parent)
  : QObject(parent)
  , m_dispatcher(dispatcher) {

  connect(&m_socket, &QUdpSocket::readyRead, this, &UdpServer::onReadyRead);
}

bool UdpServer::start(uint16_t port)
{
  if (isRunning())
    return false;

  return m_socket.bind(QHostAddress::AnyIPv4, port);
}

void UdpServer::stop()
{
  if (!isRunning())
    return;

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
  while(m_socket.hasPendingDatagrams())
  {
    QByteArray data;
    data.resize(m_socket.pendingDatagramSize());

    QHostAddress address;
    quint16 port;

    m_socket.readDatagram(
      data.data(),
      data.size(),
      &address,
      &port);

    Endpoint endpoint{
      address.toString().toStdString(),
      port
    };

    m_dispatcher.dispatch(
      std::span(
        reinterpret_cast<const std::byte*>(data.constData()),
        data.size()),
      endpoint);
  }
}

}