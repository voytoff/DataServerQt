#ifndef PACKETTYPE_H
#define PACKETTYPE_H

#include <cstdint>
#include <cstdint>

namespace qds
{

enum class PacketType : uint8_t
{
  Invalid = 0,

  SubscribeRangeRequest = 1,
  SubscribeListRequest  = 2,
  UnsubscribeRequest    = 3,
  SnapshotRequest       = 4,

  SubscribeResponse     = 20,
  SnapshotResponse      = 21,
  UnsubscribeResponse   = 22,

  Ping                  = 30,
  Pong                  = 31,

  ErrorResponse         = 40,

  LiveData              = 100
};

}

#endif // PACKETTYPE_H
