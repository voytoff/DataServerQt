#ifndef SUBSCRIBELISTREQUEST_H
#define SUBSCRIBELISTREQUEST_H

#include "datatypes.h"

namespace qds
{

struct SubscribeListRequest
{
  PublishRate rate;
  uint32_t tagCount;
};
// далее TagId[tagCount]
}

#endif // SUBSCRIBELISTREQUEST_H
