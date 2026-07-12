#include "livescheduler.h"
#include "packetwriter.h"

namespace qds
{

LiveScheduler::LiveScheduler(
  const LiveStorage& storage,
  SubscriptionManager& subscriptions,
  Publisher& publisher,
  ISender& sender)
  : m_storage(storage)
  , m_subscriptions(subscriptions)
  , m_publisher(publisher)
  , m_sender(sender)
{
}

void LiveScheduler::addSubscription(
  SubscriptionId id,
  PublishRate rate)
{
  switch (rate)
  {
  case PublishRate::Hz100:
    m_sub100Hz.push_back(id);
    break;

  case PublishRate::Hz10:
    m_sub10Hz.push_back(id);
    break;

  case PublishRate::Hz1:
    m_sub1Hz.push_back(id);
    break;
  }
}

void LiveScheduler::removeSubscription(SubscriptionId id)
{
  auto removeId =
    [id](std::vector<SubscriptionId>& v)
  {
    std::erase(v, id);
  };

  removeId(m_sub100Hz);
  removeId(m_sub10Hz);
  removeId(m_sub1Hz);
}

void LiveScheduler::tick()
{
  //   Hz100 -> every tick
  //   Hz10  -> every 10 subsequent ticks
  //   Hz1   -> every 100 subsequent ticks.

  publish(m_sub100Hz);

  if ((m_tick % 10) == 0)
    publish(m_sub10Hz);

  if (m_tick == 0)
    publish(m_sub1Hz);

  m_tick = (m_tick + 1) % 100;
}

void LiveScheduler::publish(
  const std::vector<SubscriptionId>& ids)
{
  for (SubscriptionId id : ids)
  {
    Subscription* sub =
      m_subscriptions.find(id);

    if (!sub)
      continue;

    PacketWriter writer;

    const uint64_t timestamp = 0;

    const uint32_t sequence = sub->sequence++;

    const bool ok =
      m_publisher.publish(
        m_storage,
        *sub,
        sequence,
        timestamp,
        writer);

    if (!ok)
      continue;

    m_sender.send(
      sub->endpoint,
      writer.data(),
      writer.size());
  }
}

/*
// для тестов
uint32_t LiveScheduler::publishCount() const noexcept
{
  return m_publishCount;
}
*/
}