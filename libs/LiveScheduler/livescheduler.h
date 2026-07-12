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

  void tick();

private:
  void publish(const std::vector<SubscriptionId>& ids);

private:
  const LiveStorage& m_storage;
  SubscriptionManager& m_subscriptions;
  Publisher& m_publisher;
  ISender& m_sender;

  std::vector<SubscriptionId> m_sub100Hz;
  std::vector<SubscriptionId> m_sub10Hz;
  std::vector<SubscriptionId> m_sub1Hz;

  uint32_t m_tick = 0;

/*
  // для тестов
public:
  [[nodiscard]]
  uint32_t publishCount() const noexcept;
private:
  uint32_t m_publishCount = 0;
*/
};

}

#endif // LIVESCHEDULER_H