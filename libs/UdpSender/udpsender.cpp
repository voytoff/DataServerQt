#include "udpsender.h"
#include <QString>
#include <QHostAddress>

namespace qds
{

void UdpSender::setSocket(
  QUdpSocket* socket) noexcept
{
  m_activeSocket = socket ? socket : &m_socket;
}

bool UdpSender::send(
  const Endpoint& endpoint,
  std::span<const std::byte> data)
{
  const QHostAddress address(
    QString::fromStdString(endpoint.address));

  if (address.isNull())
    return false;

  if (data.empty())
    return false;

  const qint64 sent =
    m_activeSocket->writeDatagram(
      reinterpret_cast<const char*>(data.data()),
      static_cast<qint64>(data.size()),
      address,
      endpoint.port);

  return sent == static_cast<qint64>(data.size());
}

}