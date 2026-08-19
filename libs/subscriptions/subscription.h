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

  std::vector<SignalId> signals;

  PublishRate rate = PublishRate::Hz10;

  uint32_t sequence = 0;
};

}
