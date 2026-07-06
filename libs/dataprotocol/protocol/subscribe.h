#ifndef SUBSCRIBE_H
#define SUBSCRIBE_H

#include "datatypes.h"

namespace qds
{

struct SubscribeRangeRequest
{
  TagId firstTag;
  TagId lastTag;
};
static_assert(sizeof(SubscribeRangeRequest) == 8);

}

#endif // SUBSCRIBE_H
