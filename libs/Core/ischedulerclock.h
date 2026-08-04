#pragma once

#include "datatypes.h"

namespace qds
{

class ISchedulerClock
{
public:
  virtual ~ISchedulerClock() noexcept = default;

  virtual void nextTick() noexcept = 0;

  [[nodiscard]]
  virtual FrameNumber frameNumber() const noexcept = 0;

  [[nodiscard]]
  virtual Timestamp timestamp() const noexcept = 0;

  [[nodiscard]]
  virtual WallClockTime wallClockTime() const noexcept = 0;
};

}