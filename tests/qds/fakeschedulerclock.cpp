#include "fakeschedulerclock.h"

namespace qds
{

FakeSchedulerClock::FakeSchedulerClock(
  uint64_t timestampStep,
  uint64_t wallClockStep)
  : m_timestampStep(timestampStep)
  , m_wallClockStep(wallClockStep) { }

void FakeSchedulerClock::nextTick() noexcept
{
  ++m_frame.value;
  m_timestamp.value += m_timestampStep;
  m_wallTime.unixMicroseconds += m_wallClockStep;
}

FrameNumber FakeSchedulerClock::frameNumber() const noexcept
{
  return m_frame;
}

Timestamp FakeSchedulerClock::timestamp() const noexcept
{
  return m_timestamp;
}

WallClockTime FakeSchedulerClock::wallClockTime() const noexcept
{
  return m_wallTime;
}

}
