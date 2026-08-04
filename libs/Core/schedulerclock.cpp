#include "schedulerclock.h"

namespace qds
{

SchedulerClock::SchedulerClock(IClock &clock)
  : m_clock(clock) { }

void SchedulerClock::nextTick() noexcept
{
  ++m_frame.value;
  m_timestamp = m_clock.timestamp();
  m_wallTime = m_clock.wallClockTime();
}

FrameNumber SchedulerClock::frameNumber() const noexcept
{
  return m_frame;
}

Timestamp SchedulerClock::timestamp() const noexcept
{
  return m_timestamp;
}

WallClockTime SchedulerClock::wallClockTime() const noexcept
{
  return m_wallTime;
}

}