#pragma once

#include <cstdint>
#include "datatypes.h"

namespace qds
{

struct LiveDataHeader
{
  SubscriptionId subscriptionId{};
  uint32_t sequence = 0;
  uint64_t timestamp = 0;
  uint32_t sampleCount = 0;
};

}