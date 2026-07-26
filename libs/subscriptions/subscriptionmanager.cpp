#include "subscriptionmanager.h"
#include <cstddef>
#include <algorithm>

namespace qds
{

SubscriptionId SubscriptionManager::add(const Subscription& subscription)
{
  m_subscriptions.push_back(subscription);

  m_subscriptions.back().id = SubscriptionId{m_nextId++};

  return m_subscriptions.back().id;
}

bool SubscriptionManager::remove(SubscriptionId id)
{
  auto it = std::find_if(
    m_subscriptions.begin(),
    m_subscriptions.end(),
    [id](const Subscription& sub)
    {
      return sub.id == id;
    });

  if (it == m_subscriptions.end())
    return false;

  m_subscriptions.erase(it);
  return true;
}

Subscription* SubscriptionManager::find(SubscriptionId id)
{
  return findImpl(*this, id);
}

const Subscription* SubscriptionManager::find(SubscriptionId id) const
{
  return findImpl(*this, id);
}

const std::vector<Subscription>& SubscriptionManager::subscriptions() const
{
  return m_subscriptions;
}

void SubscriptionManager::clear()
{
  m_subscriptions.clear();
  m_nextId = 1;
}

bool SubscriptionManager::empty() const
{
  return m_subscriptions.empty();
}

size_t SubscriptionManager::size() const
{
  return m_subscriptions.size();
}

} // namespace qds