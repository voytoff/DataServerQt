#ifndef PACKETDISPATCHER_H
#define PACKETDISPATCHER_H

#include "isender.h"
#include "packetreader.h"
#include "protocol/errorpackets.h"
#include "protocol/subscriptionpackets.h"
#include "subscriptionmanager.h"
#include "livescheduler.h"
#include "systemconfiguration.h"

namespace qds
{

class PacketDispatcher
{
public:

  PacketDispatcher(
    const SystemConfiguration& configuration,
    SubscriptionManager& subscriptions,
    LiveScheduler& scheduler,
    ISender& sender);

  bool dispatch(
    std::span<const std::byte> packet,
    const Endpoint& sender);

private:

  bool sendErrorResponse(
    const Endpoint& endpoint,
    ErrorCode code,
    uint32_t info = 0);


  bool processSubscribeList(
    PacketReader& reader,
    const Endpoint& endpoint);

  bool processUnsubscribe(
    PacketReader& reader,
    const Endpoint& endpoint);

  bool processPing(
    PacketReader& reader,
    const Endpoint& endpoint);

  bool sendPacket(
    const Endpoint& endpoint,
    const PacketWriter& writer);

  bool sendSubscribeResponse(
    const Endpoint& endpoint,
    SubscribeResult result,
    SubscriptionId id = {});

  bool sendUnsubscribeResponse(
    const Endpoint& endpoint,
    UnsubscribeResult result);


  bool checkEof(
    PacketReader& reader,
    const Endpoint& endpoint);

  SubscriptionId createSubscription(
    const Endpoint& endpoint,
    PublishRate rate,
    std::span<const TagId> tags);


  template<class T>
  bool readRequest(PacketReader& reader, const Endpoint& endpoint, T& value) {
    if (reader.read(value))
      return true;

    sendErrorResponse(
      endpoint,
      ErrorCode::InvalidRequest,
      reader.remaining());

    return false;
  }

private:
  const SystemConfiguration& m_configuration;
  SubscriptionManager& m_subscriptions;
  LiveScheduler& m_scheduler;
  ISender& m_sender;
};

}

#endif