#ifndef SUBSCRIPTIONS_H
#define SUBSCRIPTIONS_H

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

  std::vector<TagId> tags;

  uint32_t periodMs = 100;
};

}

#endif // SUBSCRIPTIONS_H
