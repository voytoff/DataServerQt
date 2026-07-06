#ifndef SUBSCRIPTIONMANAGER_H
#define SUBSCRIPTIONMANAGER_H

#include <vector>
#include <cstddef>
#include "subscription.h"

namespace qds
{

class SubscriptionManager
{
public:
  SubscriptionManager() = default;

  [[nodiscard]] SubscriptionId add(const Subscription& subscription);

  [[nodiscard]] bool remove(SubscriptionId id);

  [[nodiscard]] Subscription* find(SubscriptionId id);
  [[nodiscard]] const Subscription* find(SubscriptionId id) const;

  [[nodiscard]] const std::vector<Subscription>& subscriptions() const;

  void clear();

  [[nodiscard]] bool empty() const;
  [[nodiscard]] size_t size() const;

private:
  uint32_t m_nextId = 1;
  std::vector<Subscription> m_subscriptions;
};

} // namespace qds

#endif // SUBSCRIPTIONMANAGER_H
