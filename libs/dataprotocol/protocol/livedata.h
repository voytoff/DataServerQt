#ifndef LIVEDATA_H
#define LIVEDATA_H

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

#endif // LIVEDATA_H