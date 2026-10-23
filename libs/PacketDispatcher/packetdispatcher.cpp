#include "packetdispatcher.h"
#include "packetreader.h"
#include "protocol/errorpackets.h"
#include "protocol/subscriptionpackets.h"

namespace qds
{

PacketDispatcher::PacketDispatcher(
  const SystemConfiguration& configuration,
  SubscriptionManager& subscriptions,
  ISender& sender)
  : m_configuration(configuration)
  , m_subscriptions(subscriptions)
  , m_sender(sender) {}

bool PacketDispatcher::dispatch(
  std::span<const std::byte> packet,
  const Endpoint& sender)
{
  PacketReader reader;

  reader.append(packet);

  if (!reader.nextPacket())
  {
    sendErrorResponse(
      sender,
      ErrorCode::InvalidPacket);

    return false;
  }

  const PacketType type = reader.packetType();

  if (type == PacketType::SubscribeListRequest ||
      type == PacketType::UnsubscribeRequest)
  {
    RequestId requestId{};

    if (type == PacketType::SubscribeListRequest)
    {
      SubscribeListRequest req{};
      if (reader.read(req))
        requestId = req.requestId;
    }
    else
    {
      UnsubscribeRequest req{};
      if (reader.read(req))
        requestId = req.requestId;
    }

    if (requestId.value != 0)
    {
      bool handled = false;

      const bool result = tryCachedResponse(
        sender, type, requestId, packet, handled);

      if (handled)
        return result;

      m_activeRequest = ActiveRequest{
        .endpoint = sender,
        .type = type,
        .requestId = requestId,
        .data = std::vector<std::byte>(
          packet.begin(), packet.end())
      };
    }
  }

  // Обработчики должны получить reader с начала payload
  reader.clear();
  reader.append(packet);

  if (!reader.nextPacket())
    return false;

  bool result = false;

  switch (reader.packetType())
  {
  case PacketType::SubscribeListRequest:
    result = processSubscribeList(reader, sender);
    break;

  case PacketType::UnsubscribeRequest:
    result = processUnsubscribe(reader, sender);
    break;

  case PacketType::Ping:
    result = processPing(reader, sender);
    break;

  default:
    result = sendErrorResponse(
      sender, ErrorCode::UnsupportedPacket);
    break;
  }

  m_activeRequest.reset();
  return result;
}

bool PacketDispatcher::sendErrorResponse(
  const Endpoint& endpoint,
  ErrorCode code,
  uint32_t info)
{
  return reply(
    endpoint,
    PacketType::ErrorResponse,
    ErrorResponse{
      .code = code,
      .info = info
    });
}

bool PacketDispatcher::processSubscribeList(
  PacketReader &reader,
  const Endpoint &endpoint)
{
  // 1. Разбор пакета
  SubscribeListRequest req;
  if (!readRequest(reader, endpoint, req))
    return false;

  // 2. Проверка формата
  if (req.signalCount == 0) {
    sendSubscribeResponse(
      endpoint,
      req.requestId,
      SubscribeResult::EmptyList);

    return false;
  }

  if (req.signalCount > MaxSubscriptionSignals) {
    sendSubscribeResponse(
      endpoint,
      req.requestId,
      SubscribeResult::TooManySignals);

    return false;
  }

  switch (req.rate)
  {
  case PublishRate::Hz1:
  case PublishRate::Hz10:
  case PublishRate::Hz100:
    break;

  default:
    sendSubscribeResponse(
      endpoint,
      req.requestId,
      SubscribeResult::InvalidRate);
    return false;
  }


  // 3. Проверка бизнес-логики
  std::vector<SignalId> signalIds(req.signalCount);

  if (!reader.readArray(
        signalIds.data(),
        signalIds.size()))
  {
    sendErrorResponse(
      endpoint,
      ErrorCode::InvalidRequest,
      reader.remaining());

    return false;
  }

  if (!checkEof(reader, endpoint))
    return false;

  // неверный сигнал
  for (const SignalId& signalId : signalIds)
  {
    if (!m_configuration.containsSignalDefinition(signalId))
    {
      sendSubscribeResponse(
        endpoint,
        req.requestId,
        SubscribeResult::InvalidSignal);

      return false;
    }
  }

  // повторяющийся сигнал
  for (size_t i = 0; i < signalIds.size(); ++i)
  {
    for (size_t j = i + 1; j < signalIds.size(); ++j)
    {
      if (signalIds[i] == signalIds[j])
      {
        sendSubscribeResponse(
          endpoint,
          req.requestId,
          SubscribeResult::DuplicateSignal);

        return false;
      }
    }
  }

  // 4. Создание подписки
  SubscriptionId id =
    createSubscription(
      endpoint,
      req.rate,
      signalIds);

  // 5. Ответ клиенту
  return sendSubscribeResponse(
    endpoint,
    req.requestId,
    SubscribeResult::Ok,
    id);
}

bool PacketDispatcher::processUnsubscribe(PacketReader &reader, const Endpoint &endpoint)
{
  UnsubscribeRequest req;
  if (!readRequest(reader, endpoint, req))
    return false;

  if (!checkEof(reader, endpoint))
    return false;

  const Subscription* sub = m_subscriptions.find(req.id);

  if (!sub)
  {
    sendUnsubscribeResponse(
      endpoint,
      req.requestId,
      UnsubscribeResult::InvalidId);

    return false;
  }

  if (sub->endpoint != endpoint)
  {
    sendUnsubscribeResponse(
      endpoint,
      req.requestId,
      UnsubscribeResult::InvalidId);

    return false;
  }

  if (!m_subscriptions.remove(req.id))
    return false;

  return sendUnsubscribeResponse(
    endpoint,
    req.requestId,
    UnsubscribeResult::Ok);
}

bool PacketDispatcher::processPing(
  PacketReader& reader,
  const Endpoint& endpoint)
{
  if (!checkEof(reader, endpoint))
    return false;

  return reply(endpoint, PacketType::Pong);
}

bool PacketDispatcher::sendSubscribeResponse(
  const Endpoint& endpoint,
  RequestId requestId,
  SubscribeResult result,
  SubscriptionId id)
{
  return reply(
    endpoint,
    PacketType::SubscribeResponse,
    SubscribeResponse{
      .requestId = requestId,
      .result = result,
      .id = id
    });
}

bool PacketDispatcher::sendUnsubscribeResponse(
  const Endpoint &endpoint,
  RequestId requestId,
  UnsubscribeResult result)
{
  return reply(
    endpoint,
    PacketType::UnsubscribeResponse,
    UnsubscribeResponse{
      .requestId = requestId,
      .result = result,
    });
}

bool PacketDispatcher::checkEof(PacketReader &reader, const Endpoint &endpoint)
{
  if (reader.remaining() == 0 && reader.trailingBytes() == 0)
    return true;

  sendErrorResponse(
    endpoint,
    ErrorCode::ExtraData,
    reader.remaining());

  return false;
}

SubscriptionId PacketDispatcher::createSubscription(
  const Endpoint& endpoint,
  PublishRate rate,
  std::span<const SignalId> signalIds)
{
  Subscription subscription;

  subscription.endpoint = endpoint;
  subscription.rate = rate;

  subscription.signalIds.assign(
    signalIds.begin(),
    signalIds.end());

  return m_subscriptions.add(subscription);
}

void PacketDispatcher::cleanupResponseCache()
{
  const auto now = std::chrono::steady_clock::now();

  while (!m_responseCache.empty())
  {
    const auto& entry = m_responseCache.front();

    if (now - entry.timestamp < CacheLifetime)
      break;

    m_responseCache.pop_front();
  }
}

bool PacketDispatcher::tryCachedResponse(
  const Endpoint& endpoint,
  PacketType type,
  RequestId requestId,
  std::span<const std::byte> request,
  bool& handled)
{
  handled = false;

  cleanupResponseCache();

  for (const auto& entry : m_responseCache)
  {
    if (entry.endpoint != endpoint ||
        entry.type != type ||
        entry.requestId != requestId)
      continue;

    handled = true;

    if (entry.request.size() != request.size() ||
        !std::equal(
          entry.request.begin(),
          entry.request.end(),
          request.begin()))
    {
      sendErrorResponse(
        endpoint,
        ErrorCode::InvalidRequest);

      return false;
    }

    return m_sender.send(endpoint, entry.response);
  }

  return false;
}

void PacketDispatcher::saveResponse(
  std::span<const std::byte> response)
{
  if (!m_activeRequest)
    return;

  cleanupResponseCache();

  if (m_responseCache.size() >= MaxCachedResponses)
    m_responseCache.pop_front();

  const auto& req = *m_activeRequest;

  m_responseCache.push_back(CachedResponse{
    .endpoint = req.endpoint,
    .type = req.type,
    .requestId = req.requestId,
    .request = req.data,
    .response = std::vector<std::byte>(
      response.begin(), response.end()),
    .timestamp = std::chrono::steady_clock::now()
  });
}

}