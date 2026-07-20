#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <cstdint>

#include "livestorage.h"
#include "subscription.h"
#include "packetwriter.h"

namespace qds
{

class Publisher
{
public:
  bool publish(
    const LiveStorage& storage,
    const Subscription& sub,
    uint32_t sequence,
    uint64_t timestamp,
    PacketWriter& writer) const;
};

}

#endif // PUBLISHER_H