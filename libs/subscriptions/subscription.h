#pragma once

#include <cstdint>
#include <vector>

#include "datatypes.h"
#include "endpoint.h"

namespace qds
{

struct Subscription
{
  SubscriptionId id;

  Endpoint endpoint;

  std::vector<SignalId> signalIds;

  PublishRate rate = PublishRate::Hz10;

  uint32_t sequence = 0;
};

}