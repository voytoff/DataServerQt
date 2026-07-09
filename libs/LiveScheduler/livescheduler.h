#ifndef LIVESCHEDULER_H
#define LIVESCHEDULER_H
/*
#include <vector>

#include "datatypes.h"
#include "livestorage.h"
#include "subscriptionmanager.h"
#include "publisher.h"
#include "packetwriter.h"

namespace qds
{

class LiveScheduler
{
public:

  LiveScheduler(const LiveStorage& storage,
                SubscriptionManager& subscriptions,
                Publisher& publisher);

  void addSubscription(SubscriptionId id,
                       PublishRate rate);

  void removeSubscription(SubscriptionId id);

  void tick();

  const std::vector<PacketWriter>& packets() const;

private:

  void publish(
    const std::vector<SubscriptionId>& ids);

private:

  const LiveStorage& m_storage;
  SubscriptionManager& m_subscriptions;
  Publisher& m_publisher;

  std::vector<SubscriptionId> m_sub100Hz;
  std::vector<SubscriptionId> m_sub10Hz;
  std::vector<SubscriptionId> m_sub1Hz;

  std::vector<PacketWriter> m_packets;

  uint32_t m_tick = 0;
  uint32_t m_sequence = 0;
};

}
*/
#endif