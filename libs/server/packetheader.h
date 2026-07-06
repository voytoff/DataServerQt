#ifndef PACKETHEADER_H
#define PACKETHEADER_H

#include <cstdint>

namespace qds
{

#pragma pack(push,1)

struct PacketHeader
{
  uint16_t magic;        // 0xDA7A
  uint8_t version;       // 1
  uint8_t type;          // PacketType
  uint32_t payloadSize;
};

#pragma pack(pop)

}

#endif // PACKETHEADER_H
