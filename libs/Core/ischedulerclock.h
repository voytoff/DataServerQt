#pragma once

#include "iclock.h"

namespace qds
{

class ISchedulerClock : public IClock
{
public:
  ~ISchedulerClock() noexcept override = default;

  virtual void nextTick() noexcept = 0;

  [[nodiscard]]
  virtual FrameNumber frameNumber() const noexcept = 0;
};

}