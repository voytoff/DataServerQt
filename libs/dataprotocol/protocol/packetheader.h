#ifndef PACKETHEADER_H
#define PACKETHEADER_H

#include "packettype.h"
#include <cstdint>

namespace qds
{

constexpr uint16_t ProtocolMagic   = 0x4450;   // "DP"
constexpr uint8_t  ProtocolVersion = 1;
constexpr uint32_t MaxSubscriptionTags = 256;

struct PacketHeader
{
  uint16_t   magic;
  uint8_t    version;
  PacketType type;
  uint32_t   payloadSize;
};

static_assert(sizeof(PacketHeader) == 8);
}

#endif // PACKETHEADER_H
