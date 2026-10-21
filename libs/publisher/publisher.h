#pragma once

#include "frame.h"
#include "iframepublisher.h"
#include "isender.h"
#include "packetwriter.h"
#include "signalmemorylayout.h"
#include "subscriptionmanager.h"

namespace qds
{
/*
 * - Каждая подписка имеет собственный SubscriptionId и sequence.
 * - Значения передаются в порядке subscription.signalIds.
 * - Первый доступный кадр публикуется сразу.
 * - Частота определяется через PublishRate и базовую частоту кадров.
 * - При ошибке отправки sequence и расписание публикации не изменяются.
 * - В timestamp передаётся frame.timestamp.value, а не wallTime.
 */
class Publisher : public IFramePublisher
{
public:
  Publisher(
    const SignalMemoryLayout& layout,
    SubscriptionManager& subscriptions,
    ISender& sender,
    uint32_t frameRate);

  void publish(
    const Frame& frame) override;

private:

  bool publishSubscription(
    const Frame& frame,
    Subscription &subscription);

  bool shouldPublish(
    FrameNumber frame,
    const Subscription &subscription) const;

  void updateSchedule(
    FrameNumber frame,
    Subscription& subscription) const;

private:

  const SignalMemoryLayout& m_layout;
  SubscriptionManager& m_subscriptions;
  ISender& m_sender;

  PacketWriter m_writer;
  uint32_t m_frameRate = 1000;
};

}