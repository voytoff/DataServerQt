#ifndef PACKETTYPE_H
#define PACKETTYPE_H

#include <cstdint>

namespace qds
{

enum class PacketType : uint8_t
{
  SubscribeRange = 1,
  SubscribeList  = 2,

  Snapshot       = 10,

  Ping           = 100,
  Pong           = 101
};

}

#endif // PACKETTYPE_H
