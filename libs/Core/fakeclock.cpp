#include "fakeclock.h"

namespace qds
{

void FakeClock::setTimestamp(uint64_t value)
{
  m_timestamp = value;
}


void FakeClock::setWallClockTime(int64_t value)
{
  m_wallTime = value;
}


void FakeClock::advance(uint64_t microseconds)
{
  m_timestamp += microseconds;
  m_wallTime += static_cast<int64_t>(microseconds);
}


Timestamp FakeClock::timestamp() const noexcept
{
  return {m_timestamp};
}


WallClockTime FakeClock::wallClockTime() const noexcept
{
  return {m_wallTime};
}

}