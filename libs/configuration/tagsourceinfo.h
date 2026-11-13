#pragma once

#include "datatypes.h"

#include <string>

namespace qds
{

struct TagSourceInfo
{
  TagId id = {0};
  std::string name;
};

}
