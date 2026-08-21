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

  switch(reader.packetType())
  {
  case PacketType::SubscribeListRequest:
    return processSubscribeList(reader, sender);

  case PacketType::UnsubscribeRequest:
    return processUnsubscribe(reader, sender);

  case PacketType::Ping:
    return processPing(reader, sender);
  }

  sendErrorResponse(
    sender,
    ErrorCode::UnsupportedPacket);

  return false;
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

bool PacketDispatcher::processSubscribeList(PacketReader &reader, const Endpoint &endpoint)
{
  // 1. Разбор пакета
  SubscribeListRequest req;
  if (!readRequest(reader, endpoint, req))
    return false;

  // 2. Проверка формата
  if (req.signalCount == 0) {
    sendSubscribeResponse(
      endpoint,
      SubscribeResult::EmptyList);

    return false;
  }

  if (req.signalCount > MaxSubscriptionSignals) {
    sendSubscribeResponse(
      endpoint,
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
      ErrorCode::InvalidRequest);

    return false;
  }

  if (!checkEof(reader, endpoint))
    return false;

  // неверный тег
  for (const SignalId& signalId : signalIds)
  {
    if (!m_configuration.containsSignalDefinition(signalId))
    {
      sendSubscribeResponse(
        endpoint,
        SubscribeResult::InvalidSignal);

      return false;
    }
  }

  // повторяющийся тег
  for (size_t i = 0; i < signalIds.size(); ++i)
  {
    for (size_t j = i + 1; j < signalIds.size(); ++j)
    {
      if (signalIds[i] == signalIds[j])
      {
        sendSubscribeResponse(
          endpoint,
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
    sendUnsubscribeResponse(endpoint, UnsubscribeResult::InvalidId);
    return false;
  }

  /// m_scheduler.removeSubscription(req.id);

  if (!m_subscriptions.remove(req.id))
    return false;

  return sendUnsubscribeResponse(endpoint, UnsubscribeResult::Ok);
}

bool PacketDispatcher::processPing(
  PacketReader& reader,
  const Endpoint& endpoint)
{
  if (!checkEof(reader, endpoint))
    return false;

  return reply(endpoint, PacketType::Pong);
}

bool PacketDispatcher::sendSubscribeResponse(const Endpoint &endpoint, SubscribeResult result, SubscriptionId id)
{
  return reply(
    endpoint,
    PacketType::SubscribeResponse,
    SubscribeResponse{
      .result = result,
      .id = id
   });
}

bool PacketDispatcher::sendUnsubscribeResponse(const Endpoint &endpoint, UnsubscribeResult result)
{
  return reply(
    endpoint,
    PacketType::UnsubscribeResponse,
    UnsubscribeResponse{
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

}