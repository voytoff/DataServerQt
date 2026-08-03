#pragma once

#include "datatypes.h"
#include "signalstorage.h"

namespace qds
{

struct Frame
{
  FrameNumber number{};
  Timestamp timestamp{};
  WallClockTime wallTime{};

  SignalStorage storage;
};

}
