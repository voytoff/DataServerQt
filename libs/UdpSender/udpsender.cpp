#include "udpsender.h"
#include <QString>
#include <QHostAddress>

namespace qds
{

UdpSender::UdpSender()
{
}

bool UdpSender::send(
  const Endpoint& endpoint,
  const std::byte* data,
  std::size_t size)
{
  const QHostAddress address =
    QHostAddress(
      QString::fromStdString(
        endpoint.address));

  if (address.isNull())
    return false;

  if (!data || size == 0)
    return false;

  const qint64 sent =
    m_socket.writeDatagram(
      reinterpret_cast<const char*>(data),
      static_cast<qint64>(size),
      address,
      endpoint.port);

  return sent == static_cast<qint64>(size);
}

}