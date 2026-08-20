#pragma once

#include "datatypes.h"

namespace qds
{

constexpr uint32_t MaxSubscriptionTags = 256;

enum class SubscribeResult : uint8_t
{
  Ok = 0,

  InvalidRate,
  EmptyList,
  TooManyTags,
  DuplicateSignal,
  InvalidSignal
};

struct SubscribeListRequest
{
  PublishRate rate;
  uint32_t signalCount = 0;
};

struct SubscribeResponse
{
  SubscribeResult result = SubscribeResult::Ok;
  SubscriptionId id{};
};

struct UnsubscribeRequest
{
  SubscriptionId id;
};

enum class UnsubscribeResult : uint8_t
{
  Ok = 0,
  InvalidId
};

struct UnsubscribeResponse
{
  UnsubscribeResult result;
};

}