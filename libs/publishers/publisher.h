#ifndef PUBLISHER_H
#define PUBLISHER_H

#include "livestorage.h"
#include "subscription.h"
#include "packetwriter.h"

namespace qds
{

class Publisher
{
public:
  explicit Publisher(const LiveStorage& storage);

  bool publish(const Subscription& subscription,
               uint32_t sequence,
               uint64_t timestamp,
               PacketWriter& writer) const;

private:
  const LiveStorage& m_storage;
};

}

#endif // PUBLISHER_H