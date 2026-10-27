#pragma once

#include "iclock.h"
#include "ischedulerclock.h"

namespace qds
{

class SchedulerClock final : public ISchedulerClock
{
public:
  SchedulerClock(IClock& clock);

  SchedulerClock(const SchedulerClock&) = delete;
  SchedulerClock& operator=(const SchedulerClock&) = delete;


  void nextTick() noexcept override;

  // Номер кадра
  [[nodiscard]]
  FrameNumber frameNumber() const noexcept override;

  // Микросекунды
  [[nodiscard]]
  Timestamp timestamp() const noexcept override;

  // Микросекунды от Unix epoch
  [[nodiscard]]
  WallClockTime wallClockTime() const noexcept override;

private:
private:
  FrameNumber m_frame{};
  Timestamp m_timestamp{};
  WallClockTime m_wallTime{};

  IClock& m_clock;

};

}