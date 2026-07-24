#ifndef PACKETDISPATCHER_H
#define PACKETDISPATCHER_H

#include <span>
#include "endpoint.h"

namespace qds
{

class PacketDispatcher
{
public:

  bool dispatch(
    std::span<const std::byte> packet,
    const Endpoint& sender);
};

}

#endif // PACKETDISPATCHER_H
