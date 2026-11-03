#pragma once

#include <vector>

#include "datatypes.h"

namespace qds
{

struct ArchiveSignalPoint
{
  Timestamp timestamp;
  WallClockTime wallTime;
  FrameNumber frameNumber;
  double value = 0.0;
};

using ArchiveSignalPoints =
  std::vector<ArchiveSignalPoint>;

}