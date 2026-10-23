#pragma once

#include <QUdpSocket>

#include "isender.h"
#include "endpoint.h"

namespace qds
{

class UdpSender : public ISender
{
public:
  UdpSender() = default;

  void setSocket(QUdpSocket* socket) noexcept;

  bool send(
    const Endpoint& endpoint,
    std::span<const std::byte> data) override;

private:
  QUdpSocket m_socket;
  QUdpSocket* m_activeSocket = &m_socket;
};

}