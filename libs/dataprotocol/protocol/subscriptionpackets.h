#pragma once

#include "datatypes.h"

namespace qds
{

constexpr uint32_t MaxSubscriptionSignals = 256;

enum class SubscribeResult : uint8_t
{
  Ok = 0,

  InvalidRate,
  EmptyList,
  TooManySignals,
  DuplicateSignal,
  InvalidSignal
};

enum class UnsubscribeResult : uint8_t
{
  Ok = 0,
  InvalidId
};

struct SubscribeListRequest
{
  RequestId requestId;
  PublishRate rate;
  uint32_t signalCount = 0;
};

struct SubscribeResponse
{
  RequestId requestId;
  SubscribeResult result = SubscribeResult::Ok;
  SubscriptionId id{};
};

struct UnsubscribeRequest
{
  RequestId requestId;
  SubscriptionId id;
};

struct UnsubscribeResponse
{
  RequestId requestId;
  UnsubscribeResult result;
};

struct SubscriptionRenewRequest
{
  SubscriptionId id;
};

}