#ifndef SUBSCRIPTIONS_H
#define SUBSCRIPTIONS_H

#include <cstdint>
#include <vector>
#include "datatypes.h"
#include "endpoint.h"

namespace qds
{

struct Subscription {
  SubscriptionId id;
  Endpoint endpoint;
  std::vector<TagId> tags;
  PublishRate rate = PublishRate::Hz10;
  uint32_t sequence = 0;
};

}

#endif // SUBSCRIPTIONS_H
