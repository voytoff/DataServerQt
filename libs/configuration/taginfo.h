#pragma once

#include "datatypes.h"

namespace qds
{

struct TagInfo
{
  TagId tag;

  ModuleId module;
  ChannelId channel;
};

}