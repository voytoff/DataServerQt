#ifndef SUBSCRIPTIONPACKETS_H
#define SUBSCRIPTIONPACKETS_H

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
  DuplicateTag,
  InvalidTag
};

struct SubscribeListRequest
{
  PublishRate rate;
  uint32_t tagCount = 0;
};

struct SubscribeResponse
{
  SubscribeResult result = SubscribeResult::Ok;
  SubscriptionId id{};
};

}

#endif