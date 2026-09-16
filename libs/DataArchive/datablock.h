// datablock.h

#pragma once

#include "datatypes.h"

#include <cstddef>
#include <vector>

namespace qds
{

struct DataBlock
{
  ModuleId module;

  Timestamp firstTimestamp;
  WallClockTime firstWallTime;

  uint64_t framePeriodMicroseconds = 0;

  std::size_t channelCount = 0;
  std::size_t frameCount = 0;

  std::vector<double> values;
};

}