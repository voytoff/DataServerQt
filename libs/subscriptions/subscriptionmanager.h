#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <chrono>

#include "subscription.h"

namespace qds
{

class SubscriptionManager
{
public:
  SubscriptionManager() = default;

  [[nodiscard]]
  SubscriptionId add(const Subscription& subscription);

  [[nodiscard]]
  bool remove(SubscriptionId id);

  [[nodiscard]]
  Subscription* find(SubscriptionId id);

  [[nodiscard]]
  const Subscription* find(SubscriptionId id) const;

  [[nodiscard]]
  std::vector<Subscription>& subscriptions();
  [[nodiscard]]
  const std::vector<Subscription>& subscriptions() const;

  void clear();

  [[nodiscard]]
  bool empty() const;
  [[nodiscard]]
  std::size_t size() const;

  using Clock = std::chrono::steady_clock;

  static constexpr auto SubscriptionTtl =
    std::chrono::seconds{30};

  [[nodiscard]]
  bool renew(SubscriptionId id);

  std::size_t removeExpired();

private:
  SubscriptionId nextId();

  template<class Self>
  static auto findImpl(Self& self, SubscriptionId id)
  {
    auto it = std::find_if(
      self.m_subscriptions.begin(),
      self.m_subscriptions.end(),
      [id](const Subscription& sub)
      {
        return sub.id == id;
      });

    return it == self.m_subscriptions.end()
             ? nullptr
             : std::addressof(*it);
  }

private:
  uint32_t m_nextId = 1;
  std::vector<Subscription> m_subscriptions;
};

} // namespace qds
