#pragma once

#include "datatypes.h"

#include <cstdint>
#include <vector>

namespace qds
{

struct DataStreamFrame
{
  ModuleId module;

  uint64_t frameIndex = 0;

  Timestamp timestamp;
  WallClockTime wallTime;

  std::vector<double> values;
};

}