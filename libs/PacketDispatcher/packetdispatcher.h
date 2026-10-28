#pragma once

#include "isender.h"
#include "ilogger.h"
#include "packetreader.h"
#include "packetwriter.h"
#include "protocol/errorpackets.h"
#include "protocol/subscriptionpackets.h"
#include "subscriptionmanager.h"
#include "systemconfiguration.h"

#include <concepts>
#include <chrono>
#include <deque>
#include <vector>

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
    //LiveScheduler& scheduler,
    ISender& sender,
    ILogger& logger);

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

  bool processSubscriptionRenew(
    PacketReader& reader,
    const Endpoint& sender);

  bool processPing(
    PacketReader& reader,
    const Endpoint& endpoint);

  bool sendSubscribeResponse(
    const Endpoint& endpoint,
    RequestId requestId,
    SubscribeResult result,
    SubscriptionId id = {});

  bool sendUnsubscribeResponse(
    const Endpoint& endpoint, RequestId requestId,
    UnsubscribeResult result);


  bool checkEof(
    PacketReader& reader,
    const Endpoint& endpoint);

  SubscriptionId createSubscription(
    const Endpoint& endpoint,
    PublishRate rate,
    std::span<const SignalId> signalIds);


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

    if (type == PacketType::SubscribeResponse ||
        type == PacketType::UnsubscribeResponse)
    {
      saveResponse(m_writer.span());
    }

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
  struct CachedResponse
  {
    Endpoint endpoint;
    PacketType type;
    RequestId requestId;

    std::vector<std::byte> request;
    std::vector<std::byte> response;

    std::chrono::steady_clock::time_point timestamp;
  };

  static constexpr std::size_t MaxCachedResponses = 1024;
  static constexpr auto CacheLifetime = std::chrono::seconds(10);

  std::deque<CachedResponse> m_responseCache;

  struct ActiveRequest
  {
    Endpoint endpoint;
    PacketType type;
    RequestId requestId;
    std::vector<std::byte> data;
  };

  std::optional<ActiveRequest> m_activeRequest;

  void cleanupResponseCache();

  bool tryCachedResponse(
    const Endpoint& endpoint,
    PacketType type,
    RequestId requestId,
    std::span<const std::byte> request,
    bool& handled);

  void saveResponse(
    std::span<const std::byte> response);

private:
  const SystemConfiguration& m_configuration;
  SubscriptionManager& m_subscriptions;
  ISender& m_sender;
  PacketWriter m_writer;
  ILogger& m_logger;

};

}