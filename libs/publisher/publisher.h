#pragma once

#include "frame.h"
#include "iframepublisher.h"
#include "isender.h"
#include "packetwriter.h"
#include "signalmemorylayout.h"
#include "subscriptionmanager.h"

namespace qds
{

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
    PublishRate rate) const;

private:

  const SignalMemoryLayout& m_layout;
  SubscriptionManager& m_subscriptions;
  ISender& m_sender;

  PacketWriter m_writer;
  uint32_t m_frameRate = 1000;
};

}