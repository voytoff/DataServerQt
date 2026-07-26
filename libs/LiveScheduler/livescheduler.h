#ifndef LIVESCHEDULER_H
#define LIVESCHEDULER_H

#include <vector>

#include "datatypes.h"
#include "livestorage.h"
#include "subscriptionmanager.h"
#include "publisher.h"
#include "isender.h"

namespace qds
{

class LiveScheduler
{
public:
  LiveScheduler(const LiveStorage& storage,
                SubscriptionManager& subscriptions,
                Publisher& publisher,
                ISender& sender);

  void addSubscription(SubscriptionId id,
                       PublishRate rate);

  void removeSubscription(SubscriptionId id);

  [[nodiscard]] bool step();

private:
  void publish(std::span<const SubscriptionId> ids);

  template<typename Builder>
  bool send(
    const Endpoint& endpoint,
    PacketType type,
    Builder&& builder)
  {
    m_writer.begin(type);

    if (!builder(m_writer))
      return false;

    return m_sender.send(
      endpoint,
      m_writer.span());
  }

private:
  const LiveStorage& m_storage;
  SubscriptionManager& m_subscriptions;
  Publisher& m_publisher;
  ISender& m_sender;
  PacketWriter m_writer;

  std::vector<SubscriptionId> m_sub100Hz;
  std::vector<SubscriptionId> m_sub10Hz;
  std::vector<SubscriptionId> m_sub1Hz;

  uint32_t m_tick = 0;

};

}

#endif // LIVESCHEDULER_H