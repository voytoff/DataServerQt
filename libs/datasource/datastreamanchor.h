#pragma once

#include "datatypes.h"

#include <cstdint>

namespace qds
{

struct DataStreamAnchor
{
  ModuleId module;

  uint64_t firstFrameIndex = 0;

  Timestamp startTimestamp;
  WallClockTime startWallTime;

  double frameRate = 0.0;
};

}