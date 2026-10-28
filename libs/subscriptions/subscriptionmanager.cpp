#include "subscriptionmanager.h"

#include <cstddef>
#include <algorithm>

namespace qds
{

SubscriptionId SubscriptionManager::add(
  const Subscription& subscription)
{
  m_subscriptions.push_back(subscription);

  auto& sub = m_subscriptions.back();

  sub.id = nextId();
  sub.expiresAt = Clock::now() + SubscriptionTtl;

  return sub.id;
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

std::vector<Subscription> &SubscriptionManager::subscriptions()
{
  return m_subscriptions;
}

const std::vector<Subscription>& SubscriptionManager::subscriptions() const
{
  return m_subscriptions;
}

void SubscriptionManager::clear()
{
  m_subscriptions.clear();
  // ID никогда не повторяются в течение жизни процесса ?
  //m_nextId = 1;
}

bool SubscriptionManager::empty() const
{
  return m_subscriptions.empty();
}

std::size_t SubscriptionManager::size() const
{
  return m_subscriptions.size();
}

bool SubscriptionManager::renew(SubscriptionId id)
{
  Subscription* sub = find(id);

  if (!sub)
    return false;

  const auto now = Clock::now();

  if (sub->expiresAt <= now)
    return false;

  sub->expiresAt = now + SubscriptionTtl;

  return true;
}

std::size_t SubscriptionManager::removeExpired(
  const std::function<void(const Subscription&)>& onExpired)
{
  const auto now = Clock::now();

  const auto oldSize = m_subscriptions.size();

  std::erase_if(
    m_subscriptions,
    [now, &onExpired](const Subscription& sub)
    {
      if (sub.expiresAt > now)
        return false;

      if (onExpired)
        onExpired(sub);

      return true;
    });

  return oldSize - m_subscriptions.size();
}

SubscriptionId SubscriptionManager::nextId()
{
  if (m_nextId == 0)
    m_nextId = 1;

  return {m_nextId++};
}

} // namespace qds