#ifndef FRAME_H
#define FRAME_H

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

#endif // FRAME_H
