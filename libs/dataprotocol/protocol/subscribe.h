#pragma once

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