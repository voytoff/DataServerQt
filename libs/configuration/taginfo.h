#ifndef TAGINFO_H
#define TAGINFO_H

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

#endif // TAGINFO_H
