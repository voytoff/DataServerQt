#include "publisher.h"

#include "protocol/publishheader.h"

namespace qds
{

Publisher::Publisher(
  const SignalMemoryLayout& layout,
  SubscriptionManager& subscriptions,
  ISender& sender, uint32_t frameRate)
  : m_layout(layout)
  , m_subscriptions(subscriptions)
  , m_sender(sender)
  , m_frameRate(frameRate)
{
}

void Publisher::publish(const Frame& frame)
{
  for (Subscription& subscription :
       m_subscriptions.subscriptions())
  {
    if (!shouldPublish(
          frame.number,
          subscription.rate))
    {
      continue;
    }

    if (!publishSubscription(
          frame,
          subscription))
    {
      // TODO: logging/statistics
    }
  }
}

bool Publisher::publishSubscription(
  const Frame& frame,
  Subscription& subscription)
{
  m_writer.begin(
    PacketType::LiveData);

  PublishHeader header{};

  header.subscriptionId =
    subscription.id;

  // Следующий sequence.
  // Увеличим его только после успешной отправки.
  header.sequence =
    subscription.sequence + 1;

  header.timestamp =
    frame.timestamp.value;

  header.valueCount =
    static_cast<uint32_t>(
      subscription.signalIds.size());

  m_writer.write(header);

  for (const SignalId signal :
       subscription.signalIds)
  {
    const SignalReference reference =
      m_layout.reference(signal);

    if (!reference.isValid())
      return false;

    const double* value =
      frame.address(reference);

    if (value == nullptr)
      return false;

    m_writer.write(*value);
  }

  if (!m_sender.send(
        subscription.endpoint,
        m_writer.span()))
  {
    return false;
  }

  // Только после успешной отправки.
  subscription.sequence =
    header.sequence;

  return true;
}

bool Publisher::shouldPublish(
  FrameNumber frame,
  PublishRate rate) const
{
  const uint32_t frequency =
    static_cast<uint32_t>(rate);

  if (frequency == 0 ||
      frequency > m_frameRate)
  {
    return false;
  }

  const uint64_t period =
    m_frameRate / frequency;

  return ((frame.value - 1) % period) == 0;
}

}