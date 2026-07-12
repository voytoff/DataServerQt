#ifndef UDPSENDER_H
#define UDPSENDER_H

#include <QUdpSocket>
#include "isender.h"
#include "endpoint.h"

namespace qds
{

class UdpSender : public ISender
{
public:
  UdpSender();

  bool send(const Endpoint& endpoint,
            const std::byte* data,
            std::size_t size) override;

private:
  QUdpSocket m_socket;
};

}

#endif // UDPSENDER_H