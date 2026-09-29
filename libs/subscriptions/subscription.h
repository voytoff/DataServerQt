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
  PublishRate rate;

  std::vector<SignalId> signalIds;

  uint32_t sequence = 0;

  FrameNumber nextPublishFrame;
  bool publishStarted = false;
};

}