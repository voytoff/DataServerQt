#pragma once

#include <cstdint>

#include "livestorage.h"
#include "subscription.h"
#include "packetwriter.h"

namespace qds
{

// строит LiveData пакет
class Publisher
{
public:
  bool publish(
    const LiveStorage& storage,
    const Subscription& sub,
    uint32_t sequence,
    PacketWriter& writer) const;

};

}