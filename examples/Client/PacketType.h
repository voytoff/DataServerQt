#ifndef PACKETTYPE_H
#define PACKETTYPE_H

#include <cstdint>

namespace qds
{

struct PacketHeader
{
  uint16_t magic = 0xDA7A;
  uint16_t type;
  uint32_t size;
};

enum class PacketType : uint16_t
{
  SubscribeRange = 1,
  SubscribeList  = 2,
  UnsubscribeAll = 3,
  Snapshot       = 10
};

}

#endif // PACKETTYPE_H
