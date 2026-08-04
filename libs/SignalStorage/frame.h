#pragma once

#include "datatypes.h"
#include "signalmemory.h"

namespace qds
{

struct Frame
{
  FrameNumber number;
  Timestamp timestamp;
  WallClockTime wallTime;

  RawMemory raw;
  CalculatedMemory calculated;
};

}
