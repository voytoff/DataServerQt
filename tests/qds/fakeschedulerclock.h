#pragma once

#include "ischedulerclock.h"

namespace qds
{

class FakeSchedulerClock : public ISchedulerClock
{
public:
  FakeSchedulerClock(
    uint64_t timestampStep = 10,
    uint64_t wallClockStep = 100);

  void nextTick() noexcept override;

  [[nodiscard]]
  FrameNumber frameNumber() const noexcept override;

  [[nodiscard]]
  Timestamp timestamp() const noexcept override;

  [[nodiscard]]
  WallClockTime wallClockTime() const noexcept override;

private:
  FrameNumber m_frame{0};
  Timestamp m_timestamp{0};
  WallClockTime m_wallTime{0};

  uint64_t m_timestampStep;
  uint64_t m_wallClockStep;

};

}