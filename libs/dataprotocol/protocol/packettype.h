#ifndef PACKETTYPE_H
#define PACKETTYPE_H

#include <cstdint>
#include <cstdint>

namespace qds
{

enum class PacketType : uint8_t {
  Invalid = 0,

  SubscribeRange = 1,
  SubscribeList  = 2,
  Unsubscribe    = 3,

  Snapshot       = 10,

  Ping           = 20,
  Pong           = 21,

  Error          = 30,
  LiveData       = 100
};

}

#endif // PACKETTYPE_H
