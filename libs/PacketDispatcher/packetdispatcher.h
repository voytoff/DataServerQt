#pragma once

#include "isender.h"
#include "packetreader.h"
#include "protocol/errorpackets.h"
#include "protocol/subscriptionpackets.h"
#include "subscriptionmanager.h"
#include "livescheduler.h"
#include "systemconfiguration.h"
#include <concepts>

namespace qds
{

template<typename T>
concept PacketBuilder =
  std::invocable<T&, PacketWriter&>;

// принимает команды
// создает подписки
// отвечает клиенту
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

  template<typename T>
    requires std::is_trivially_copyable_v<T>
  bool reply(
    const Endpoint& endpoint,
    PacketType type,
    const T& packet)
  {
    m_writer.begin(type);
    m_writer.write(packet);

    return m_sender.send(endpoint, m_writer.span());
  }

  // reply без payload
  bool reply(
    const Endpoint& endpoint,
    PacketType type)
  {
    m_writer.begin(type);

    return m_sender.send(
      endpoint,
      m_writer.span());
  }

  template<PacketBuilder Builder>
  bool reply(
    const Endpoint& endpoint,
    PacketType type,
    Builder&& builder)
  {
    m_writer.begin(type);

    builder(m_writer);

    return m_sender.send(
      endpoint,
      m_writer.span());
  }

private:
  const SystemConfiguration& m_configuration;
  SubscriptionManager& m_subscriptions;
  LiveScheduler& m_scheduler;
  ISender& m_sender;
  PacketWriter m_writer;

};

}